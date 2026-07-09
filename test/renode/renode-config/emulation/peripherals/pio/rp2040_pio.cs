using Antmicro.Renode.Core;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using Antmicro.Renode.Logging;

using Antmicro.Renode.Time;
using ELFSharp.ELF;

using Antmicro.Renode.Peripherals.Miscellaneous;
using Antmicro.Renode.Utilities.Binding;
using Antmicro.Migrant;
using Antmicro.Renode.Peripherals.Bus;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace Antmicro.Renode.Peripherals.CPU
{
    // Delegate types for native PIO simulator bindings
    public delegate void ActionInt32(int arg);
    public delegate uint FuncInt32UInt32UInt32(int arg1, uint arg2);
    public delegate void ActionInt32UInt32UInt32(int arg1, uint arg2, uint arg3);

    public static class PioSimPathExtension
    {
        public static void CreateSegmentDisplayTester(this Emulation emulation, string name)
        {
            var piosimPath = new PioSimPath();
            emulation.ExternalsManager.AddExternal(piosimPath, name);
        }
    }

    public class PioSimPath : IExternal
    {
        public string path;
    }
    // parts of this class can be left unmodified;
    // to integrate an external simulator you need to
    // look for comments in the code below
    public class RP2040PIOCPU : BaseCPU, IRP2040Peripheral, IGPIOReceiver, ITimeSink, IDisposable, IDoubleWordPeripheral, IKnownSize, INumberedGPIOOutput
    {
        private static string GetSourceFileDirectory([CallerFilePath] string sourceFilePath = "")
        {
            return Path.GetDirectoryName(sourceFilePath);
        }

        private static string GetPioSimPath()
        {
            return Path.GetFullPath(GetSourceFileDirectory() + "/../../../piosim/");
        }

        private static string GetPioSimLibraryPath(string basepath)
        {
            string libraryName;
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                // for windows there is delivered compiled version of piosim
                // environment to compile that is quite hard to setup correctly
                libraryName = "libpiosim.dll";
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                libraryName = "libpiosim.dylib";
            }
            else
            {
                libraryName = "libpiosim.so";
            }
            return Path.GetFullPath(basepath + "/" + libraryName);
        }

        public RP2040PIOCPU(string cpuType, IMachine machine, ulong address, GPIOPort.RP2040GPIO gpio, uint id, RP2040Clocks clocks,
            Endianess endianness = Endianess.LittleEndian, CpuBitness bitness = CpuBitness.Bits32)
            : base(id + 100, cpuType, machine, endianness, bitness)
        {
            pioId = (int)id;
            // Get the directory of the executing assembly
            string piosimPath = "";
            if (EmulationManager.Instance.CurrentEmulation.ExternalsManager.TryGetByName("piosim_path", out PioSimPath result))
            {
                piosimPath = result.path;
            }
            else
            {
                piosimPath = GetPioSimPath();
            }
            string libraryFile = GetPioSimLibraryPath(piosimPath);
            binder = new NativeBinder(this, libraryFile);
            machine.GetSystemBus(this).Register(this, new BusRangeRegistration(new Antmicro.Renode.Core.Range(address, (ulong)Size)));
            this.gpio = gpio;
            gpioFunction = id == 0 ? GPIOPort.RP2040GPIO.GpioFunction.PIO0 : GPIOPort.RP2040GPIO.GpioFunction.PIO1;
            machine.GetSystemBus(this).Register(this, new BusMultiRegistration(address + xorAliasOffset, aliasSize, "XOR"));
            machine.GetSystemBus(this).Register(this, new BusMultiRegistration(address + setAliasOffset, aliasSize, "SET"));
            machine.GetSystemBus(this).Register(this, new BusMultiRegistration(address + clearAliasOffset, aliasSize, "CLEAR"));
            gpio.ReevaluatePioActions.Add((uint steps) =>
            {
                totalExecutedInstructions += PioExecute(pioId, steps);
                UpdateSignals();
            });
            clocks.OnSystemClockChange(UpdateClocks);

            var innerConnections = new Dictionary<int, IGPIO>();
            for (int i = 0; i < 10; i++)
            {
                innerConnections[i] = new GPIO();
            }
            Connections = new ReadOnlyDictionary<int, IGPIO>(innerConnections);

            this.Log(LogLevel.Info, "PIO{0} successfuly created!", id);
        }

        public IReadOnlyDictionary<int, IGPIO> Connections { get; }

        private void UpdateSignals()
        {
            // PIO Registers
            const uint FSTAT = 0x004;
            const uint INTS0 = 0x12c;
            const uint INTS1 = 0x138;

            uint fstat = PioReadMemory(pioId, FSTAT);
            uint ints0 = PioReadMemory(pioId, INTS0);
            uint ints1 = PioReadMemory(pioId, INTS1);

            // Connections:
            // 0: IRQ0, 1: IRQ1
            // 2-5: TX DREQ (0-3), 6-9: RX DREQ (0-3)
            Connections[0].Set((ints0 & 0xF) != 0);
            Connections[1].Set((ints1 & 0xF) != 0);

            for (int i = 0; i < 4; i++)
            {
                // TX FIFO Not Full (DREQ)
                Connections[2 + i].Set((fstat & (1u << (16 + i))) == 0);
                // RX FIFO Not Empty (DREQ)
                Connections[6 + i].Set((fstat & (1u << (8 + i))) == 0);
            }
        }

        [ConnectionRegion("XOR")]
        public virtual void WriteDoubleWordXor(long offset, uint value)
        {
            PioWriteMemory(pioId, (uint)offset, PioReadMemory(pioId, (uint)offset) ^ value);
            UpdateSignals();
        }

        [ConnectionRegion("SET")]
        public virtual void WriteDoubleWordSet(long offset, uint value)
        {
            PioWriteMemory(pioId, (uint)offset, PioReadMemory(pioId, (uint)offset) | value);
            UpdateSignals();
        }

        [ConnectionRegion("CLEAR")]
        public virtual void WriteDoubleWordClear(long offset, uint value)
        {
            PioWriteMemory(pioId, (uint)offset, PioReadMemory(pioId, (uint)offset) & (~value));
            UpdateSignals();
        }

        [ConnectionRegion("XOR")]
        public virtual uint ReadDoubleWordXor(long offset)
        {
            return PioReadMemory(pioId, (uint)offset);
        }

        [ConnectionRegion("SET")]
        public virtual uint ReadDoubleWordSet(long offset)
        {
            return PioReadMemory(pioId, (uint)offset);
        }

        [ConnectionRegion("CLEAR")]
        public virtual uint ReadDoubleWordClear(long offset)
        {
            return PioReadMemory(pioId, (uint)offset);
        }

        private void UpdateClocks(long systemClockFrequency)
        {
            uint newPerformance = (uint)(systemClockFrequency / 1000000);
            if (newPerformance == 0)
            {
                newPerformance = 1;
            }
            this.PerformanceInMips = newPerformance;
            this.Log(LogLevel.Debug, "Changing clock frequency to: {0} MIPS", newPerformance);
        }

        public override void Start()
        {
            base.Start();

            PioInitialize(pioId);
        }

        public override void Reset()
        {
            base.Reset();

            instructionsExecutedThisRound = 0;
            totalExecutedInstructions = 0;
            PioReset(pioId);
            UpdateSignals();
            // [Here goes an invocation resetting the external simulator (if needed)]
            // [This can be used to revert the internal state of the simulator to the initial form]
        }

        public virtual uint ReadDoubleWord(long offset)
        {
            lock (this)
            {
                uint val = (uint)PioReadMemory(pioId, (uint)offset);
                UpdateSignals();
                return val;
            }
        }

        public virtual void WriteDoubleWord(long offset, uint value)
        {
            lock (this)
            {
                PioWriteMemory(pioId, (uint)offset, (uint)value);
                UpdateSignals();
            }
        }

        public override void Dispose()
        {
            lock (this)
            {
                PioDeinitialize(pioId);

                base.Dispose();
            }
            // [Here goes an invocation disposing the external simulator (if needed)]
            // [This can be used to clean all unmanaged resources used to communicate with the simulator]
        }

        public void OnGPIO(int number, bool value)
        {
            if (!IsStarted)
            {
                return;
            }

            // [Here goes an invocation triggering an IRQ in the external simulator]
        }

        public virtual void SetRegisterValue32(int register, uint value)
        {
            // [Here goes an invocation setting the register value in the external simulator]
        }

        public virtual uint GetRegisterValue32(int register)
        {
            // [Here goes an invocation reading the register value from the external simulator]
            return 0;
        }

        public override ExecutionResult ExecuteInstructions(ulong numberOfInstructionsToExecute, out ulong numberOfExecutedInstructions)
        {
            try
            {
                // [Here comes the invocation of the external simulator for the given amount of instructions]
                // [This is the place where simulation of acutal instructions is to be executed]
                lock (this)
                {
                    instructionsExecutedThisRound += (ulong)PioExecute(pioId, (uint)numberOfInstructionsToExecute);
                }
            }
            catch (Exception)
            {
                this.NoisyLog("CPU exception detected, halting.");
                //InvokeHalted(new HaltArguments(HaltReason.Abort, this));
                instructionsExecutedThisRound = 0UL;
                return ExecutionResult.Aborted;
            }
            finally
            {
                numberOfExecutedInstructions = instructionsExecutedThisRound;
                totalExecutedInstructions += instructionsExecutedThisRound;
                instructionsExecutedThisRound = 0UL;
            }

            return ExecutionResult.Ok;
        }

        public override string Architecture => "RP2040_PIO";

        public override RegisterValue PC
        {
            get
            {
                return GetRegisterValue32(PCRegisterId);
            }

            set
            {
                SetRegisterValue32(PCRegisterId, value);
            }
        }

        [Export]
        protected virtual void LogAsCpu(int level, string s)
        {
            this.Log((LogLevel)level, s);
        }

        [Export]
        protected virtual void GpioPinWriteBitset(uint bitset, uint bitmap)
        {
            gpio.ClearGpioBitset((~bitset) & bitmap, gpioFunction);
            gpio.SetGpioBitset(bitset, gpioFunction, bitmap);
        }

        [Export]
        protected virtual void GpioPindirWriteBitset(uint bitset, uint bitmap)
        {
            gpio.SetPinDirectionBitset(bitset, bitmap);
        }

        [Export]
        protected virtual int GpioGetPinState(uint pin)
        {
            return Convert.ToInt32(this.gpio.GetGpioState(pin));
        }

        [Export]
        protected virtual uint GetGpioPinBitmap()
        {
            return (uint)this.gpio.GetGpioStateBitmap();
        }

        private GPIOPort.RP2040GPIO gpio;

        public override ulong ExecutedInstructions => totalExecutedInstructions;

        private ulong instructionsExecutedThisRound;
        private ulong totalExecutedInstructions;
        // [This needs to be mapped to the id of the Program Counter register used by the simulator]
        private const int PCRegisterId = 0;
        private int pioId;
        private GPIOPort.RP2040GPIO.GpioFunction gpioFunction;

        [Transient]
        private NativeBinder binder;

        public long Size { get { return 0x1000; } }
        public const ulong aliasSize = 0x1000;
        public const ulong xorAliasOffset = 0x1000;
        public const ulong setAliasOffset = 0x2000;
        public const ulong clearAliasOffset = 0x3000;

        [Import]
        private ActionInt32 PioInitialize;

        [Import]
        private ActionInt32 PioReset;

        [Import]
        private ActionInt32 PioDeinitialize;

        [Import]
        private FuncInt32UInt32UInt32 PioExecute;

        [Import]
        private ActionInt32UInt32UInt32 PioWriteMemory;

        [Import]
        private FuncInt32UInt32UInt32 PioReadMemory;
    }
}
