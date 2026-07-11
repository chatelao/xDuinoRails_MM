/**
 * motor_model.cs
 *
 * Copyright (c) 2024 Jules
 *
 * Distributed under the terms of the MIT License.
 */

using System;
using Antmicro.Renode.Core;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Analog;
using Antmicro.Renode.Time;
using Antmicro.Renode.Peripherals;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class MotorModel : IGPIOReceiver, IExternal
    {
        public MotorModel(IMachine machine)
        {
            this.machine = machine;

            // Default parameters
            this.Kv = 1000;         // RPM/V
            this.Vbus = 12.0;       // Volts
            this.Resistance = 0.1;  // Ohms
            this.Inertia = 0.0001;  // kg*m^2
            this.Friction = 0.0001; // N*m*s
            this.LoadTorque = 0.0;  // N*m
            this.Vstart = 0.0;      // Volts
            this.Vstop = 0.0;       // Volts

            this.updateThread = machine.ObtainManagedThread(UpdateAction, 1000); // 1kHz simulation
            Reset();
        }

        public void OnGPIO(int number, bool value)
        {
            lock(sync)
            {
                // Immediate update before changing state to capture old state physics
                UpdateInternal();
                pwmState = value;
                // Update again to reflect new state in ADC immediately
                UpdateInternal();
            }
        }

        private void UpdateAction()
        {
            lock(sync)
            {
                UpdateInternal();
            }
        }

        private void UpdateInternal()
        {
            var now = machine.ElapsedVirtualTime.TimeElapsed;
            double dt = (now - lastUpdateTime).TotalSeconds;

            if (dt > 0)
            {
                double omega = velocity;
                double Ke = 60.0 / (2.0 * Math.PI * Kv);
                double Vapplied = pwmState ? Vbus : 0;

                if (velocity <= 0.01 && Vapplied < Vstart)
                {
                    Vapplied = 0;
                }
                else if (velocity > 0.01 && Vapplied < Vstop)
                {
                    Vapplied = 0;
                }

                // d_omega = (Ke/R * Vapplied - (Ke^2/R + Friction) * omega - LoadTorque) / Inertia * dt
                double acceleration = (Ke / Resistance * Vapplied - (Ke * Ke / Resistance + Friction) * omega - LoadTorque) / Inertia;
                velocity += acceleration * dt;
                if (velocity < 0) velocity = 0;

                lastUpdateTime = now;
            }

            double Ke_current = 60.0 / (2.0 * Math.PI * Kv);
            double Vbemf = velocity * Ke_current;
            double Vadc = pwmState ? Vbus : Vbemf;

            // Scaled for ADC (assuming 3.3V max)
            double Vmapped = Vadc * 3.3 / 12.0;
            if (Vmapped > 3.3) Vmapped = 3.3;
            if (Vmapped < 0) Vmapped = 0;

            if (Adc != null)
            {
                Adc.SetDefaultVoltageOnChannel(AdcChannel, Vmapped);
            }

            if (Encoder != null)
            {
                Encoder.Velocity = velocity;
            }
        }

        public void Reset()
        {
            lock(sync)
            {
                velocity = 0;
                pwmState = false;
                lastUpdateTime = machine.ElapsedVirtualTime.TimeElapsed;
                updateThread.Start();
            }
        }

        public double Kv { get; set; }
        public double Vbus { get; set; }
        public double Resistance { get; set; }
        public double Inertia { get; set; }
        public double Friction { get; set; }
        public double LoadTorque { get; set; }
        public double Vstart { get; set; }
        public double Vstop { get; set; }
        public RP2040ADC Adc { get; set; }
        public int AdcChannel { get; set; }
        public QuadratureEncoder Encoder { get; set; }

        public double Velocity => velocity;

        private readonly IMachine machine;
        private readonly IManagedThread updateThread;
        private readonly object sync = new object();
        private double velocity; // rad/s
        private bool pwmState;
        private TimeInterval lastUpdateTime;
    }
}
