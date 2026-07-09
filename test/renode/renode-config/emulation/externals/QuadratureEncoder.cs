/**
 * QuadratureEncoder.cs
 *
 * Copyright (c) 2024 Jules
 *
 * Distributed under the terms of the MIT License.
 */

using System;
using System.Collections.Generic;
using Antmicro.Renode.Core;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Time;
using Antmicro.Renode.Peripherals;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class QuadratureEncoder : IExternal, INumberedGPIOOutput
    {
        public QuadratureEncoder(IMachine machine)
        {
            this.machine = machine;
            this.A = new GPIO();
            this.B = new GPIO();
            this.Resolution = 1000; // pulses per revolution (PPR)

            this.updateThread = machine.ObtainManagedThread(UpdateAction, 10000); // 10kHz simulation for high speed pulses
            Reset();
        }

        public IReadOnlyDictionary<int, IGPIO> Connections => new Dictionary<int, IGPIO>
        {
            { 0, A },
            { 1, B }
        };

        private void UpdateAction()
        {
            lock(sync)
            {
                var now = machine.ElapsedVirtualTime.TimeElapsed;
                double dt = (now - lastUpdateTime).TotalSeconds;

                if (dt > 0)
                {
                    // Total counts per second = Velocity (rad/s) / (2*PI) * Resolution (PPR) * 4 (Quadrature)
                    double countsPerSecond = (Velocity / (2.0 * Math.PI)) * Resolution * 4.0;
                    positionAccumulator += countsPerSecond * dt;

                    int currentIntPos = (int)Math.Floor(positionAccumulator);
                    if (currentIntPos != lastIntPos)
                    {
                        // Position changed, update outputs
                        // Gray code sequence for quadrature: 00 -> 01 -> 11 -> 10 -> ...
                        UpdateOutputs(currentIntPos);
                        lastIntPos = currentIntPos;
                    }
                    lastUpdateTime = now;
                }
            }
        }

        private void UpdateOutputs(int pos)
        {
            int step = pos % 4;
            if (step < 0) step += 4;

            bool nextA = (step == 1 || step == 2);
            bool nextB = (step == 2 || step == 3);

            if (A.IsSet != nextA) A.Set(nextA);
            if (B.IsSet != nextB) B.Set(nextB);
        }

        public void Reset()
        {
            lock(sync)
            {
                positionAccumulator = 0;
                lastIntPos = 0;
                Velocity = 0;
                lastUpdateTime = machine.ElapsedVirtualTime.TimeElapsed;
                A.Set(false);
                B.Set(false);
                updateThread.Start();
            }
        }

        public double Velocity { get; set; } // rad/s
        public double Resolution { get; set; } // PPR

        public GPIO A { get; }
        public GPIO B { get; }

        private readonly IMachine machine;
        private readonly IManagedThread updateThread;
        private readonly object sync = new object();
        private double positionAccumulator;
        private int lastIntPos;
        private TimeInterval lastUpdateTime;
    }
}
