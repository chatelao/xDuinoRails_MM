/**
 * rp2040_pwm.cs
 *
 * Copyright (c) 2024 Jules
 *
 * Distributed under the terms of the MIT License.
 */

using System;
using System.Collections.Generic;
using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;
using Antmicro.Renode.Time;
using Antmicro.Renode.Utilities;
using Antmicro.Renode.Peripherals.Timers;

namespace Antmicro.Renode.Peripherals.PWM
{
    public class RP2040PWM : RP2040PeripheralBase, INumberedGPIOOutput, IGPIOReceiver
    {
        public RP2040PWM(IMachine machine, ulong address) : base(machine, address)
        {
            IRQ = new GPIO();
            var innerConnections = new Dictionary<int, IGPIO>();
            PWMOut = new GPIO[NumberOfSlices * 2];
            for (int i = 0; i < PWMOut.Length; i++)
            {
                PWMOut[i] = new GPIO();
                innerConnections[i] = PWMOut[i];
            }

            // DREQs 24-31
            for (int i = 0; i < NumberOfSlices; i++)
            {
                innerConnections[24 + i] = new GPIO();
            }

            Connections = innerConnections;

            slices = new PWMSlice[NumberOfSlices];
            for (int i = 0; i < NumberOfSlices; ++i)
            {
                slices[i] = new PWMSlice(this, i);
            }
            DefineRegisters();
            Reset();
        }

        private void DefineRegisters()
        {
            Registers.CH0_CSR.DefineMany(this, NumberOfSlices, (reg, index) =>
            {
                reg.WithFlag(0, out slices[index].enabled, name: $"CH{index}_CSR_EN", writeCallback: (old, val) => {
                        if (val && !old) {
                            slices[index].OnEnable();
                        }
                        slices[index].Update();
                   })
                   .WithFlag(1, out slices[index].phaseCorrect, name: $"CH{index}_CSR_PH_CORRECT", writeCallback: (_, __) => slices[index].Update())
                   .WithFlag(2, out slices[index].invertA, name: $"CH{index}_CSR_A_INV", writeCallback: (_, __) => slices[index].Update())
                   .WithFlag(3, out slices[index].invertB, name: $"CH{index}_CSR_B_INV", writeCallback: (_, __) => slices[index].Update())
                   .WithValueField(4, 2, out slices[index].divMode, name: $"CH{index}_CSR_DIVMODE", writeCallback: (_, __) => slices[index].Update())
                   .WithFlag(6, FieldMode.Write, name: $"CH{index}_CSR_PH_RET", writeCallback: (_, val) => { if(val) slices[index].RetardPhase(); })
                   .WithFlag(7, FieldMode.Write, name: $"CH{index}_CSR_PH_ADV", writeCallback: (_, val) => { if(val) slices[index].AdvancePhase(); })
                   .WithReservedBits(8, 24);
            }, stepInBytes: 0x14);

            Registers.CH0_DIV.DefineMany(this, NumberOfSlices, (reg, index) =>
            {
                reg.WithValueField(0, 4, out slices[index].divFrac, name: $"CH{index}_DIV_FRAC", writeCallback: (_, __) => slices[index].Update())
                   .WithValueField(4, 8, out slices[index].divInt, name: $"CH{index}_DIV_INT", writeCallback: (_, __) => slices[index].Update())
                   .WithReservedBits(12, 20);
            }, stepInBytes: 0x14);

            Registers.CH0_CTR.DefineMany(this, NumberOfSlices, (reg, index) =>
            {
                reg.WithValueField(0, 16, name: $"CH{index}_CTR",
                    valueProviderCallback: _ => (uint)slices[index].GetCurrentCounter(),
                    writeCallback: (_, val) => slices[index].SetCounter((ushort)val))
                   .WithReservedBits(16, 16);
            }, stepInBytes: 0x14);

            Registers.CH0_CC.DefineMany(this, NumberOfSlices, (reg, index) =>
            {
                reg.WithValueField(0, 16, out slices[index].shadowCompareA, name: $"CH{index}_CC_A")
                   .WithValueField(16, 16, out slices[index].shadowCompareB, name: $"CH{index}_CC_B");
            }, stepInBytes: 0x14);

            Registers.CH0_TOP.DefineMany(this, NumberOfSlices, (reg, index) =>
            {
                reg.WithValueField(0, 16, out slices[index].shadowTop, name: $"CH{index}_TOP")
                   .WithReservedBits(16, 16);
            }, stepInBytes: 0x14);

            Registers.EN.Define(this)
                .WithValueField(0, 8, writeCallback: (_, val) =>
                {
                    for (int i = 0; i < 8; i++)
                    {
                        bool newVal = BitHelper.IsBitSet(val, (byte)i);
                        if (newVal && !slices[i].enabled.Value) {
                            slices[i].OnEnable();
                        }
                        slices[i].enabled.Value = newVal;
                        slices[i].Update();
                    }
                }, valueProviderCallback: _ =>
                {
                    uint val = 0;
                    for (int i = 0; i < 8; i++)
                    {
                        if (slices[i].enabled.Value) val |= (1u << i);
                    }
                    return val;
                }, name: "EN");

            Registers.INTR.Define(this)
                .WithValueField(0, 8, out interruptsRaw, name: "INTR", writeCallback: (_, val) => {
                    interruptsRaw.Value &= ~val;
                    UpdateInterrupts();
                })
                .WithReservedBits(8, 24);
            Registers.INTE.Define(this)
                .WithValueField(0, 8, out interruptsEnabled, name: "INTE", writeCallback: (_, __) => UpdateInterrupts())
                .WithReservedBits(8, 24);
            Registers.INTF.Define(this)
                .WithValueField(0, 8, out interruptsForce, name: "INTF", writeCallback: (_, val) => {
                    interruptsForce.Value = val;
                    UpdateInterrupts();
                })
                .WithReservedBits(8, 24);
            Registers.INTS.Define(this)
                .WithValueField(0, 8, FieldMode.Read, valueProviderCallback: _ => (interruptsRaw.Value | interruptsForce.Value) & interruptsEnabled.Value, name: "INTS")
                .WithReservedBits(8, 24);
        }

        public override void Reset()
        {
            base.Reset();
            foreach (var slice in slices)
            {
                slice.Reset();
            }
            UpdateInterrupts();
        }

        public void SetInterrupt(int slice)
        {
            interruptsRaw.Value |= (1u << slice);
            UpdateInterrupts();
        }

        public void SendDreq(int slice)
        {
            IGPIO dreq;
            if (Connections.TryGetValue(24 + slice, out dreq))
            {
                dreq.Set(true);
                dreq.Set(false);
            }
        }

        public void ExternalTrigger(int slice, bool value)
        {
            if (slice < 0 || slice >= NumberOfSlices) return;
            this.Log(LogLevel.Info, "ExternalTrigger: slice={0}, value={1}", slice, value);
            slices[slice].OnInput(value);
        }

        public void OnGPIO(int number, bool value)
        {
            // We'll treat the signal number as the slice index.
            if (number < 0 || number >= NumberOfSlices)
            {
                this.Log(LogLevel.Error, "Received GPIO signal on unsupported input {0}", number);
                return;
            }
            this.Log(LogLevel.Info, "OnGPIO: number={0}, value={1}", number, value);
            slices[number].OnInput(value);
        }

        private void UpdateInterrupts()
        {
            bool status = ((interruptsRaw.Value | interruptsForce.Value) & interruptsEnabled.Value) != 0;
            IRQ.Set(status);
        }

        public double GetDutyCycleA(int slice) => slices[slice].DutyCycleA;
        public double GetDutyCycleB(int slice) => slices[slice].DutyCycleB;
        public double GetFrequency(int slice) => slices[slice].Frequency;

        public GPIO IRQ { get; private set; }
        public IReadOnlyDictionary<int, IGPIO> Connections { get; }
        private GPIO[] PWMOut { get; }

        private readonly PWMSlice[] slices;
        private const int NumberOfSlices = 8;

        private IValueRegisterField interruptsRaw;
        private IValueRegisterField interruptsEnabled;
        private IValueRegisterField interruptsForce;

        private class PWMSlice
        {
            public PWMSlice(RP2040PWM parent, int index)
            {
                this.parent = parent;
                this.index = index;
                // We use a 2GHz internal frequency (125MHz * 16) to handle fractional dividers
                timer = new LimitTimer(parent.machine.ClockSource, 2000000000, parent, $"pwm{index}", limit: 0xFFFF, eventEnabled: true, autoUpdate: true);
                timer.LimitReached += OnWrap;
            }

            public void Reset()
            {
                timer.Reset();
                activeCompareA = 0;
                activeCompareB = 0;
                activeTop = 0xFFFF;
                lastInputState = false;
                Update();
            }

            public void OnEnable()
            {
                // Latch shadow registers when enabled
                Latch();
            }

            public void Latch()
            {
                activeCompareA = (uint)shadowCompareA.Value;
                activeCompareB = (uint)shadowCompareB.Value;
                activeTop = (uint)shadowTop.Value;
                parent.Log(LogLevel.Info, "PWM Slice {0}: Latched CC=0x{1:X8}, TOP=0x{2:X4}", index, (activeCompareB << 16) | activeCompareA, activeTop);
                Update();
            }

            public void AdvancePhase()
            {
                parent.Log(LogLevel.Info, "PWM Slice {0}: AdvancePhase. Enabled={1}", index, enabled.Value);
                if (!enabled.Value) return;
                IncrementCounter();
            }

            public void RetardPhase()
            {
                parent.Log(LogLevel.Info, "PWM Slice {0}: RetardPhase. Enabled={1}", index, enabled.Value);
                if (!enabled.Value) return;
                if (timer.Value > 0)
                {
                    timer.Value--;
                }
                else
                {
                    timer.Value = (ulong)activeTop;
                }
                parent.Log(LogLevel.Info, "PWM Slice {0}: RetardPhase. New Value={1}", index, timer.Value);
            }

            public void OnInput(bool value)
            {
                bool oldState = lastInputState;
                lastInputState = value;

                parent.Log(LogLevel.Info, "PWM Slice {0}: OnInput({1}). OldState={2}, Enabled={3}, DivMode={4}", index, value, oldState, enabled.Value, divMode.Value);

                if (!enabled.Value) return;

                switch ((uint)divMode.Value)
                {
                    case 1: // LEVEL
                        timer.Enabled = value;
                        break;
                    case 2: // RISE
                        if (value && !oldState) IncrementCounter();
                        break;
                    case 3: // FALL
                        if (!value && oldState) IncrementCounter();
                        break;
                }
            }

            private void IncrementCounter()
            {
                var newValue = (timer.Value + 1) % ((ulong)activeTop + 1);
                timer.Value = newValue;
                if (newValue == 0)
                {
                    OnWrap();
                }
                parent.Log(LogLevel.Info, "PWM Slice {0}: IncrementCounter. New Value={1}", index, timer.Value);
            }

            private void OnWrap()
            {
                Latch();
                parent.SetInterrupt(index);
                parent.SendDreq(index);
            }

            public void Update()
            {
                if (!enabled.Value)
                {
                    timer.Enabled = false;
                    parent.PWMOut[2 * index].Set(invertA.Value);
                    parent.PWMOut[2 * index + 1].Set(invertB.Value);
                    return;
                }

                uint mode = (uint)divMode.Value;
                parent.Log(LogLevel.Info, "PWM Slice {0}: Updating. Mode={1}, Enabled={2}, TOP={3}", index, mode, enabled.Value, activeTop);
                if (mode == 0) // FREE RUNNING
                {
                    timer.Enabled = true;
                }
                else if (mode == 1) // LEVEL
                {
                    timer.Enabled = lastInputState;
                }
                else // RISE or FALL
                {
                    timer.Enabled = false; // Manual increment in OnInput
                }

                uint divider = (uint)(divInt.Value * 16 + divFrac.Value);
                if (divider == 0) divider = 16; // Minimum divider is 1.0 (16/16)

                // The error was: Cannot implicitly convert type 'int' to 'ulong' for timer.Divider
                timer.Divider = (ulong)divider;
                timer.Limit = (ulong)activeTop + 1;

                double topVal = (double)activeTop + 1;
                double dcA = (double)activeCompareA / topVal;
                double dcB = (double)activeCompareB / topVal;

                if (dcA > 1.0) dcA = 1.0;
                if (dcB > 1.0) dcB = 1.0;

                if (invertA.Value) dcA = 1.0 - dcA;
                if (invertB.Value) dcB = 1.0 - dcB;

                double divisor = divider / 16.0;
                double period = phaseCorrect.Value ? (2.0 * activeTop) : (activeTop + 1.0);
                if (period == 0) period = 1.0;

                double freq = 125000000.0 / divisor / period;

                parent.Log(LogLevel.Info, "PWM Slice {0}: Frequency {1}Hz, Duty A {2:P}, Duty B {3:P}", index, freq, dcA, dcB);

                // Update GPIO state based on Duty Cycle > 0 for basic visualization
                parent.PWMOut[2 * index].Set(dcA > 0);
                parent.PWMOut[2 * index + 1].Set(dcB > 0);
            }

            public uint GetCurrentCounter()
            {
                if (timer.Value >= (ulong)activeTop + 1) return 0;
                return (uint)timer.Value;
            }
            public void SetCounter(ushort val)
            {
                timer.Value = (ulong)val;
                parent.Log(LogLevel.Info, "PWM Slice {0}: Counter manually set to {1}", index, val);
            }

            public double DutyCycleA => (double)activeCompareA / (activeTop + 1);
            public double DutyCycleB => (double)activeCompareB / (activeTop + 1);
            public double Frequency
            {
                get
                {
                    double divisor = divInt.Value + (divFrac.Value / 16.0);
                    if (divisor == 0) divisor = 1.0;
                    double freq = 125000000.0 / divisor / (activeTop + 1);
                    if (phaseCorrect.Value) freq /= 2.0;
                    return freq;
                }
            }

            public IFlagRegisterField enabled;
            public IValueRegisterField divMode;
            public IFlagRegisterField phaseCorrect;
            public IFlagRegisterField invertA;
            public IFlagRegisterField invertB;
            public IValueRegisterField divFrac;
            public IValueRegisterField divInt;

            public IValueRegisterField shadowCompareA;
            public IValueRegisterField shadowCompareB;
            public IValueRegisterField shadowTop;

            public uint activeCompareA;
            public uint activeCompareB;
            public uint activeTop;

            private bool lastInputState;
            private readonly LimitTimer timer;
            private readonly RP2040PWM parent;
            private readonly int index;
        }

        private enum Registers
        {
            CH0_CSR = 0x00,
            CH0_DIV = 0x04,
            CH0_CTR = 0x08,
            CH0_CC = 0x0c,
            CH0_TOP = 0x10,
            // Slices 1-7 follow at 0x14 intervals
            EN = 0xa0,
            INTR = 0xa4,
            INTE = 0xa8,
            INTF = 0xac,
            INTS = 0xb0
        }
    }
}
