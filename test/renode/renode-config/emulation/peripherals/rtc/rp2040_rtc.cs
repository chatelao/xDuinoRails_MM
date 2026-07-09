/**
 * rp2040_rtc.cs
 *
 * Copyright (c) 2024 Jules
 *
 * Distributed under the terms of the MIT License.
 */

using System;
using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Timers;
using Antmicro.Renode.Time;

namespace Antmicro.Renode.Peripherals.Timers
{
    public class RP2040RTC : RP2040PeripheralBase, IKnownSize
    {
        public RP2040RTC(IMachine machine, ulong address) : base(machine, address)
        {
            IRQ = new GPIO();
            // clk_rtc is usually 46875Hz (from 12MHz XOSC / 256)
            // The default CLKDIV_M1 is 46874 to get a 1Hz tick.
            // We'll use a timer that fires every 1 second.
            tickTimer = new LimitTimer(machine.ClockSource, 1, this, "rtc_tick", limit: 1, direction: Direction.Ascending, enabled: false, workMode: WorkMode.Periodic, eventEnabled: true);
            tickTimer.LimitReached += OnTick;

            DefineRegisters();
            Reset();
        }

        public override void Reset()
        {
            base.Reset();
            tickTimer.Enabled = false;
            rtcActive = false;
            rtcEnabled = false;
            forceNotLeapYear = false;

            year = 0;
            month = 1;
            day = 1;
            dotw = 0;
            hour = 0;
            minute = 0;
            second = 0;

            matchActive = false;
            matchEnabled = false;
            yearMatchEnabled = false;
            monthMatchEnabled = false;
            dayMatchEnabled = false;
            dotwMatchEnabled = false;
            hourMatchEnabled = false;
            minMatchEnabled = false;
            secMatchEnabled = false;

            irqYear = 0;
            irqMonth = 0;
            irqDay = 0;
            irqDotw = 0;
            irqHour = 0;
            irqMin = 0;
            irqSec = 0;

            rawInterrupt = false;
            interruptEnabled = false;
            interruptForce = false;

            UpdateInterrupts();
        }

        private void OnTick()
        {
            second++;
            if (second >= 60)
            {
                second = 0;
                minute++;
                if (minute >= 60)
                {
                    minute = 0;
                    hour++;
                    if (hour >= 24)
                    {
                        hour = 0;
                        dotw = (dotw + 1) % 7;
                        day++;
                        if (day > DaysInMonth(month, year))
                        {
                            day = 1;
                            month++;
                            if (month > 12)
                            {
                                month = 1;
                                year++;
                            }
                        }
                    }
                }
            }

            CheckAlarm();
        }

        private int DaysInMonth(int m, int y)
        {
            switch (m)
            {
                case 1: return 31;
                case 2: return IsLeapYear(y) ? 29 : 28;
                case 3: return 31;
                case 4: return 30;
                case 5: return 31;
                case 6: return 30;
                case 7: return 31;
                case 8: return 31;
                case 9: return 30;
                case 10: return 31;
                case 11: return 30;
                case 12: return 31;
                default: return 31;
            }
        }

        private bool IsLeapYear(int y)
        {
            if (forceNotLeapYear) return false;
            // Datasheet: "If the current value of YEAR in SETUP_0 is evenly divisible by 4, a leap year is detected"
            // "Since this is not always true (century years for example), the leap year checking can be forced off"
            return (y % 4 == 0);
        }

        private void CheckAlarm()
        {
            bool match = false;
            if (matchEnabled)
            {
                match = true;
                if (yearMatchEnabled && year != irqYear) match = false;
                if (monthMatchEnabled && month != irqMonth) match = false;
                if (dayMatchEnabled && day != irqDay) match = false;
                if (dotwMatchEnabled && dotw != irqDotw) match = false;
                if (hourMatchEnabled && hour != irqHour) match = false;
                if (minMatchEnabled && minute != irqMin) match = false;
                if (secMatchEnabled && second != irqSec) match = false;
            }

            if (match != rawInterrupt)
            {
                rawInterrupt = match;
                UpdateInterrupts();
            }
        }

        private void UpdateInterrupts()
        {
            bool status = (rawInterrupt || interruptForce) && interruptEnabled;
            IRQ.Set(status);
        }

        private void LoadFromSetup()
        {
            year = setupYear;
            month = setupMonth;
            day = setupDay;
            dotw = setupDotw;
            hour = setupHour;
            minute = setupMin;
            second = setupSec;
            this.Log(LogLevel.Info, "RTC Loaded: {0}-{1:D2}-{2:D2} {3:D2}:{4:D2}:{5:D2} (DOTW {6})", year, month, day, hour, minute, second, dotw);
            CheckAlarm();
        }

        private void DefineRegisters()
        {
            Registers.CLKDIV_M1.Define(this)
                .WithValueField(0, 16, out clkDivM1, name: "CLKDIV_M1");

            Registers.SETUP_0.Define(this)
                .WithValueField(0, 5, writeCallback: (_, val) => setupDay = (int)val, valueProviderCallback: _ => (ulong)setupDay, name: "DAY")
                .WithReservedBits(5, 3)
                .WithValueField(8, 4, writeCallback: (_, val) => setupMonth = (int)val, valueProviderCallback: _ => (ulong)setupMonth, name: "MONTH")
                .WithValueField(12, 12, writeCallback: (_, val) => setupYear = (int)val, valueProviderCallback: _ => (ulong)setupYear, name: "YEAR")
                .WithReservedBits(24, 8);

            Registers.SETUP_1.Define(this)
                .WithValueField(0, 6, writeCallback: (_, val) => setupSec = (int)val, valueProviderCallback: _ => (ulong)setupSec, name: "SEC")
                .WithReservedBits(6, 2)
                .WithValueField(8, 6, writeCallback: (_, val) => setupMin = (int)val, valueProviderCallback: _ => (ulong)setupMin, name: "MIN")
                .WithReservedBits(14, 2)
                .WithValueField(16, 5, writeCallback: (_, val) => setupHour = (int)val, valueProviderCallback: _ => (ulong)setupHour, name: "HOUR")
                .WithReservedBits(21, 3)
                .WithValueField(24, 3, writeCallback: (_, val) => setupDotw = (int)val, valueProviderCallback: _ => (ulong)setupDotw, name: "DOTW")
                .WithReservedBits(27, 5);

            Registers.CTRL.Define(this)
                .WithFlag(0, writeCallback: (_, val) => {
                    rtcEnabled = val;
                    tickTimer.Enabled = val;
                    rtcActive = val;
                    this.Log(LogLevel.Info, "RTC Enable: {0}", val);
                }, valueProviderCallback: _ => rtcEnabled, name: "RTC_ENABLE")
                .WithFlag(1, FieldMode.Read, valueProviderCallback: _ => rtcActive, name: "RTC_ACTIVE")
                .WithReservedBits(2, 2)
                .WithFlag(4, FieldMode.Write, writeCallback: (_, val) => {
                    if (val) LoadFromSetup();
                }, name: "LOAD")
                .WithReservedBits(5, 3)
                .WithFlag(8, writeCallback: (_, val) => forceNotLeapYear = val, valueProviderCallback: _ => forceNotLeapYear, name: "FORCE_NOTLEAPYEAR")
                .WithReservedBits(9, 23);

            Registers.IRQ_SETUP_0.Define(this)
                .WithValueField(0, 5, writeCallback: (_, val) => { irqDay = (int)val; CheckAlarm(); }, valueProviderCallback: _ => (ulong)irqDay, name: "DAY")
                .WithReservedBits(5, 3)
                .WithValueField(8, 4, writeCallback: (_, val) => { irqMonth = (int)val; CheckAlarm(); }, valueProviderCallback: _ => (ulong)irqMonth, name: "MONTH")
                .WithValueField(12, 12, writeCallback: (_, val) => { irqYear = (int)val; CheckAlarm(); }, valueProviderCallback: _ => (ulong)irqYear, name: "YEAR")
                .WithFlag(24, writeCallback: (_, val) => { dayMatchEnabled = val; CheckAlarm(); }, valueProviderCallback: _ => dayMatchEnabled, name: "DAY_ENA")
                .WithFlag(25, writeCallback: (_, val) => { monthMatchEnabled = val; CheckAlarm(); }, valueProviderCallback: _ => monthMatchEnabled, name: "MONTH_ENA")
                .WithFlag(26, writeCallback: (_, val) => { yearMatchEnabled = val; CheckAlarm(); }, valueProviderCallback: _ => yearMatchEnabled, name: "YEAR_ENA")
                .WithReservedBits(27, 1)
                .WithFlag(28, writeCallback: (_, val) => {
                    matchEnabled = val;
                    matchActive = val;
                    this.Log(LogLevel.Info, "RTC Match Enable: {0}", val);
                    CheckAlarm();
                }, valueProviderCallback: _ => matchEnabled, name: "MATCH_ENA")
                .WithFlag(29, FieldMode.Read, valueProviderCallback: _ => matchActive, name: "MATCH_ACTIVE")
                .WithReservedBits(30, 2);

            Registers.IRQ_SETUP_1.Define(this)
                .WithValueField(0, 6, writeCallback: (_, val) => { irqSec = (int)val; CheckAlarm(); }, valueProviderCallback: _ => (ulong)irqSec, name: "SEC")
                .WithReservedBits(6, 2)
                .WithValueField(8, 6, writeCallback: (_, val) => { irqMin = (int)val; CheckAlarm(); }, valueProviderCallback: _ => (ulong)irqMin, name: "MIN")
                .WithReservedBits(14, 2)
                .WithValueField(16, 5, writeCallback: (_, val) => { irqHour = (int)val; CheckAlarm(); }, valueProviderCallback: _ => (ulong)irqHour, name: "HOUR")
                .WithReservedBits(21, 3)
                .WithValueField(24, 3, writeCallback: (_, val) => { irqDotw = (int)val; CheckAlarm(); }, valueProviderCallback: _ => (ulong)irqDotw, name: "DOTW")
                .WithReservedBits(27, 1)
                .WithFlag(28, writeCallback: (_, val) => { secMatchEnabled = val; CheckAlarm(); }, valueProviderCallback: _ => secMatchEnabled, name: "SEC_ENA")
                .WithFlag(29, writeCallback: (_, val) => { minMatchEnabled = val; CheckAlarm(); }, valueProviderCallback: _ => minMatchEnabled, name: "MIN_ENA")
                .WithFlag(30, writeCallback: (_, val) => { hourMatchEnabled = val; CheckAlarm(); }, valueProviderCallback: _ => hourMatchEnabled, name: "HOUR_ENA")
                .WithFlag(31, writeCallback: (_, val) => { dotwMatchEnabled = val; CheckAlarm(); }, valueProviderCallback: _ => dotwMatchEnabled, name: "DOTW_ENA");

            Registers.RTC_1.Define(this)
                .WithValueField(0, 5, FieldMode.Read, valueProviderCallback: _ => (ulong)latchedDay, name: "DAY")
                .WithReservedBits(5, 3)
                .WithValueField(8, 4, FieldMode.Read, valueProviderCallback: _ => (ulong)latchedMonth, name: "MONTH")
                .WithValueField(12, 12, FieldMode.Read, valueProviderCallback: _ => (ulong)latchedYear, name: "YEAR")
                .WithReservedBits(24, 8);

            Registers.RTC_0.Define(this)
                .WithValueField(0, 6, FieldMode.Read, valueProviderCallback: _ => {
                    latchedYear = year;
                    latchedMonth = month;
                    latchedDay = day;
                    return (ulong)second;
                }, name: "SEC")
                .WithReservedBits(6, 2)
                .WithValueField(8, 6, FieldMode.Read, valueProviderCallback: _ => (ulong)minute, name: "MIN")
                .WithReservedBits(14, 2)
                .WithValueField(16, 5, FieldMode.Read, valueProviderCallback: _ => (ulong)hour, name: "HOUR")
                .WithReservedBits(21, 3)
                .WithValueField(24, 3, FieldMode.Read, valueProviderCallback: _ => (ulong)dotw, name: "DOTW")
                .WithReservedBits(27, 5);

            Registers.INTR.Define(this)
                .WithFlag(0, FieldMode.Read, valueProviderCallback: _ => rawInterrupt, name: "RTC")
                .WithReservedBits(1, 31);

            Registers.INTE.Define(this)
                .WithFlag(0, writeCallback: (_, val) => {
                    interruptEnabled = val;
                    UpdateInterrupts();
                }, valueProviderCallback: _ => interruptEnabled, name: "RTC")
                .WithReservedBits(1, 31);

            Registers.INTF.Define(this)
                .WithFlag(0, writeCallback: (_, val) => {
                    interruptForce = val;
                    UpdateInterrupts();
                }, valueProviderCallback: _ => interruptForce, name: "RTC")
                .WithReservedBits(1, 31);

            Registers.INTS.Define(this)
                .WithFlag(0, FieldMode.Read, valueProviderCallback: _ => (rawInterrupt || interruptForce) && interruptEnabled, name: "RTC")
                .WithReservedBits(1, 31);
        }

        public GPIO IRQ { get; private set; }

        private IValueRegisterField clkDivM1;
        private readonly LimitTimer tickTimer;

        private bool rtcEnabled;
        private bool rtcActive;
        private bool forceNotLeapYear;

        private int year;
        private int month;
        private int day;
        private int dotw;
        private int hour;
        private int minute;
        private int second;

        private int latchedYear;
        private int latchedMonth;
        private int latchedDay;

        private int setupYear;
        private int setupMonth;
        private int setupDay;
        private int setupDotw;
        private int setupHour;
        private int setupMin;
        private int setupSec;

        private bool matchActive;
        private bool matchEnabled;
        private bool yearMatchEnabled;
        private bool monthMatchEnabled;
        private bool dayMatchEnabled;
        private bool dotwMatchEnabled;
        private bool hourMatchEnabled;
        private bool minMatchEnabled;
        private bool secMatchEnabled;

        private int irqYear;
        private int irqMonth;
        private int irqDay;
        private int irqDotw;
        private int irqHour;
        private int irqMin;
        private int irqSec;

        private bool rawInterrupt;
        private bool interruptEnabled;
        private bool interruptForce;

        private enum Registers
        {
            CLKDIV_M1 = 0x00,
            SETUP_0 = 0x04,
            SETUP_1 = 0x08,
            CTRL = 0x0c,
            IRQ_SETUP_0 = 0x10,
            IRQ_SETUP_1 = 0x14,
            RTC_1 = 0x18,
            RTC_0 = 0x1c,
            INTR = 0x20,
            INTE = 0x24,
            INTF = 0x28,
            INTS = 0x2c
        }
    }
}
