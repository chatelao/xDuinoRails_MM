using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using System;
using Antmicro.Renode.Time;
using Antmicro.Renode.Peripherals.Miscellaneous;

namespace Antmicro.Renode.Peripherals.Timers
{
    public class RP2040Timer : RP2040PeripheralBase, IKnownSize
    {
        public class Alarm
        {
            public bool IrqEnabled { get; set; }
            public GPIO Irq { get; set; }
            public bool RawInterrupt { get; set; }

            public LimitTimer Clock { get; private set; }
            public uint AlarmLimit { get; private set; }

            private readonly RP2040Timer parent;

            public Alarm(RP2040Timer timer, IMachine machine, int id)
            {
                parent = timer;
                Irq = new GPIO();
                IrqEnabled = false;
                RawInterrupt = false;

                Clock = new LimitTimer(machine.ClockSource, 1000000, timer, "AlarmTimer" + id, direction: Direction.Ascending, enabled: false, workMode: WorkMode.OneShot, eventEnabled: true, autoUpdate: true)
                {
                    AutoUpdate = true,
                    Value = 0
                };
                Clock.LimitReached += OnCounterFired;
            }

            public void Reset()
            {
                Irq.Unset();
                IrqEnabled = false;
                RawInterrupt = false;
                Clock.Enabled = false;
                Clock.Value = 0;
                Clock.Limit = 0;
                AlarmLimit = 0;
            }

            public void Enable(bool value)
            {
                Clock.Enabled = value;
            }
            public void SetAlarm(ulong currentTicks, ulong limit)
            {
                AlarmLimit = (uint)limit;
                uint currentLow = (uint)currentTicks;
                uint limitLow = (uint)limit;

                ulong ticksToWait;
                if(limitLow >= currentLow)
                {
                    ticksToWait = limitLow - currentLow;
                }
                else
                {
                    ticksToWait = (0x100000000UL - currentLow) + limitLow;
                }

                // Ensure we wait at least one tick to avoid instant firing issues in some engines
                Clock.Limit = Math.Max(1, ticksToWait);
                Clock.Value = 0;
                RawInterrupt = false;
                UpdateInterrupt();
            }

            public void UpdateInterrupt()
            {
                bool masked = RawInterrupt && IrqEnabled;
                if (masked)
                {
                    Irq.Set(true);
                }
                else
                {
                    Irq.Unset();
                }
            }

            private void OnCounterFired()
            {
                RawInterrupt = true;
                Clock.Enabled = false;
                UpdateInterrupt();
            }
        }

        private enum Registers
        {
            TIMEHR = 0x08,
            TIMELR = 0x0c,
            ALARM0 = 0x10,
            ALARM1 = 0x14,
            ALARM2 = 0x18,
            ALARM3 = 0x1c,
            ARMED = 0x20,
            TIMERAWH = 0x24,
            TIMERAWL = 0x28,
            INTR = 0x34,
            INTE = 0x38,
            INTF = 0x3c,
            INTS = 0x40
        }

        Alarm[] alarms;
        private uint latchedHigh;

        public RP2040Timer(Machine machine, ulong address) : base(machine, address)
        {
            IRQs = new GPIO[4];
            for (int i = 0; i < 4; ++i)
            {
                IRQs[i] = new GPIO();
            }
            alarms = new Alarm[4];
            for (int i = 0; i < alarms.Length; ++i)
            {
                alarms[i] = new Alarm(this, machine, i)
                {
                    Irq = IRQs[i]
                };
            }

            Clock = new LimitTimer(machine.ClockSource, 1000000, this, "SystemClock", limit: 0xffffffffffffffff, direction: Direction.Ascending, eventEnabled: false, enabled: true, workMode: WorkMode.Periodic);
            DefineRegisters();
            Reset();
        }

        public override void Reset()
        {
            for (int i = 0; i < IRQs.Length; ++i)
            {
                IRQs[i].Unset();
            }
            for (int i = 0; i < alarms.Length; ++i)
            {
                alarms[i].Reset();
            }
            latchedHigh = 0;
        }
        public void DefineRegisters()
        {
            Registers.TIMERAWH.Define(this)
                .WithValueField(0, 32, FieldMode.Read,
                    valueProviderCallback: _ => (Clock.Value >> 32) & 0xffffffff,
                    name: "TIMERAWH");

            Registers.TIMERAWL.Define(this)
                .WithValueField(0, 32, FieldMode.Read,
                    valueProviderCallback: _ => Clock.Value & 0xffffffff,
                    name: "TIMERAWL");

            Registers.TIMEHR.Define(this)
                .WithValueField(0, 32, FieldMode.Read,
                    valueProviderCallback: _ => latchedHigh,
                    name: "TIMEHR");

            Registers.TIMELR.Define(this)
                .WithValueField(0, 32, FieldMode.Read,
                    valueProviderCallback: _ =>
                    {
                        latchedHigh = (uint)(Clock.Value >> 32);
                        return (uint)(Clock.Value & 0xffffffff);
                    },
                    name: "TIMELR");

            Registers.INTR.Define(this)
                .WithFlags(0, 4, FieldMode.Read | FieldMode.WriteOneToClear,
                    valueProviderCallback: (i, _) => alarms[i].RawInterrupt,
                    writeCallback: (i, _, value) =>
                    {
                        if (value)
                        {
                            alarms[i].RawInterrupt = false;
                            alarms[i].UpdateInterrupt();
                        }
                    },
                    name: "INTR");

            Registers.ARMED.Define(this)
                .WithFlags(0, 4, FieldMode.Read | FieldMode.Write,
                    valueProviderCallback: (i, _) => alarms[i].Clock.Enabled,
                    writeCallback: (i, _, value) =>
                    {
                        if (value)
                        {
                            alarms[i].Enable(false);
                        }
                    },
                    name: "ARMED");

            Registers.INTS.Define(this)
                .WithFlags(0, 4, FieldMode.Read,
                    valueProviderCallback: (i, _) => alarms[i].RawInterrupt && alarms[i].IrqEnabled,
                    name: "INTS");

            Registers.INTE.Define(this)
                .WithFlags(0, 4, FieldMode.Read | FieldMode.Write,
                    valueProviderCallback: (i, _) => alarms[i].IrqEnabled,
                    writeCallback: (i, _, value) =>
                    {
                        alarms[i].IrqEnabled = value;
                        alarms[i].UpdateInterrupt();
                    },
                    name: "INTE");

            Registers.INTF.Define(this)
                .WithFlags(0, 4, FieldMode.Read | FieldMode.Write,
                    valueProviderCallback: (i, _) => false, // Always returns 0 on read
                    writeCallback: (i, _, value) =>
                    {
                        if (value)
                        {
                            alarms[i].RawInterrupt = true;
                            alarms[i].UpdateInterrupt();
                        }
                    },
                    name: "INTF");

            int alarmNumber = 0;
            foreach (Registers r in Enum.GetValues(typeof(Registers)))
            {
                if (r >= Registers.ALARM0 && r <= Registers.ALARM3)
                {
                    int id = alarmNumber;
                    r.Define(this)
                        .WithValueField(0, 32, FieldMode.Write | FieldMode.Read,
                            writeCallback: (_, val) =>
                            {
                                alarms[id].SetAlarm(Clock.Value, val);
                                alarms[id].Enable(true);
                            },
                            valueProviderCallback: _ => (ulong)alarms[id].AlarmLimit,
                            name: "ALARM" + id);
                    alarmNumber++;
                }
            }
        }
        public GPIO[] IRQs { get; private set; }
        public GPIO IRQ0 => IRQs[0];
        public GPIO IRQ1 => IRQs[1];
        public GPIO IRQ2 => IRQs[2];
        public GPIO IRQ3 => IRQs[3];

        private LimitTimer Clock;
    }
}
