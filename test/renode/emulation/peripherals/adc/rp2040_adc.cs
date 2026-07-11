/**
 * rp2040_adc.cs
 *
 * Copyright (c) 2024 Mateusz Stadnik <matgla@live.com>
 *
 * Distributed under the terms of the MIT License.
 */

using System;
using System.Linq;
using System.Collections.Generic;
using Antmicro.Renode.Core;
using Antmicro.Renode.Exceptions;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Utilities;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Peripherals.Bus;
using Antmicro.Renode.Peripherals;
using Antmicro.Renode.Peripherals.Sensors;
using Antmicro.Renode.Peripherals.Miscellaneous;
using Antmicro.Renode.Utilities.Collections;
using Antmicro.Renode.Peripherals.GPIOPort;
using Antmicro.Renode.Utilities.RESD;

namespace Antmicro.Renode.Peripherals.Analog
{
  public class RP2040ADC : RP2040PeripheralBase
  {
    public RP2040ADC(IMachine machine, RP2040Clocks clocks, RP2040Pads pads, ulong address) : base(machine, address)
    {
      this.IRQ = new GPIO();
      this.fifo = new CircularBuffer<ushort>(fifoSize);
      this.sampleProvider = new SensorSamplesFifo<ScalarSample>[channelsCount];
      this.resdStream = new RESDStream<VoltageSample>[channelsCount];
      this.defaultSample = new ScalarSample[channelsCount];
      for (int channel = 0; channel < channelsCount; ++channel)
      {
        sampleProvider[channel] = new SensorSamplesFifo<ScalarSample>();
        resdStream[channel] = null;
        defaultSample[channel] = new ScalarSample(0);
      }
      clocks.OnAdcChange(UpdateFrequency);
      this.samplingThread = machine.ObtainManagedThread(Sample, 1);
      this.samplingThread.Stop();
      this.pads = pads;
      this.DMARequest = new GPIO();
      DefineRegisters();
      Reset();
    }

    public void FeedVoltageSampleToChannel(int channel, string path)
    {
      if (channel < 0 || channel >= channelsCount)
      {
        this.Log(LogLevel.Error, "Provided sample for non-existing channel: " + channel);
        return;
      }
      sampleProvider[channel].FeedSamplesFromFile(path);
    }

    public void SetOnboardTemperature(double temperature)
    {
      onboardTemperature = temperature;
    }

    public void SetDefaultVoltageOnChannel(int channel, double voltage)
    {
      if (channel < 0 || channel >= channelsCount)
      {
        this.Log(LogLevel.Error, "Provided sample for non-existing channel: " + channel);
        return;
      }
      var sample = new ScalarSample((decimal)voltage);
      defaultSample[channel] = sample;
    }
    public void FeedVoltageSampleToChannel(int channel, decimal valueInV, uint repeat)
    {
      if (channel < 0 || channel >= channelsCount)
      {
        this.Log(LogLevel.Error, "Provided sample for non-existing channel: " + channel);
        return;
      }
      var sample = new ScalarSample(valueInV);
      for (int i = 0; i < repeat; ++i)
      {
        sampleProvider[channel].FeedSample(sample);
      }
    }

    public void FeedVoltageSampleToChannel(int channel, decimal startValueInV, decimal stepDeltaInV, uint repeatSample, uint numberOfSamples)
    {
      if (channel < 0 || channel >= channelsCount)
      {
        this.Log(LogLevel.Error, "Provided sample for non-existing channel: " + channel);
        return;
      }

      for (uint i = 0; i < numberOfSamples; ++i)
      {
        FeedVoltageSampleToChannel(channel, startValueInV + i * stepDeltaInV, repeatSample);
      }
    }

    public void FeedSamplesFromRESD(ReadFilePath filePath, uint channel, uint resdChannel = 0,
        RESDStreamSampleOffset sampleOffsetType = RESDStreamSampleOffset.Specified, long sampleOffsetTime = 0)
    {
      if (channel < 0 || channel >= channelsCount)
      {
        return;
      }
      try
      {
        resdStream[channel] = this.CreateRESDStream<VoltageSample>(filePath, resdChannel, sampleOffsetType, sampleOffsetTime);
      }
      catch (RESDException)
      {
        for (var channelId = 0; channelId < channelsCount; channelId++)
        {
          if (resdStream[channelId] != null)
          {
            resdStream[channelId].Dispose();
          }
        }
        throw new RecoverableException($"Could not load RESD channel {resdChannel} from {filePath}");
      }
    }

    public override void Reset()
    {
      dreqEnabled = false;
      state = State.Waiting;
      time = 0;
      enabled = false;
      ready = false;
      temperatureSensorEnabled = false;
      trigger = Trigger.Nothing;
      error = false;
      errorSticky = false;
      selectedInput = 0;
      roundRobinChannels = 0;
      conversionResult = 0;
      fifoEnabled = false;
      fifoShift = false;
      fifoError = false;
      fifoAssertDMA = false;
      fifoThreshold = 0;
      fifoOverflowed = false;
      fifoUnderflowed = false;

      dividerIntegral = 0;
      dividerFrac = 0;

      fifoIrqEnabled = false;
      fifoIrqForced = false;
      running = false;
      samplingThread.Stop();

      onboardTemperature = 25.5;
    }

    private void Sample()
    {
      switch (state)
      {
        case State.Waiting:
          {
            ready = true;
            if (trigger != Trigger.Nothing)
            {
              this.Log(LogLevel.Noisy, "ADC State: Waiting -> Sampling");
              state = State.Sampling;
              time = 0;
              ready = false;
              error = false;
            }
            return;
          }
        case State.Delay:
          {
            ready = true;
            time += 1;
            decimal interval = 1 + dividerIntegral + (decimal)dividerFrac / 256;
            decimal delay = interval - sampleTime;
            if (time >= (int)delay)
            {
              this.Log(LogLevel.Noisy, "ADC State: Delay -> Sampling");
              state = State.Sampling;
              time = 0;
              ready = false;
              error = false;
            }
            return;
          }
        case State.Sampling:
          {
            ready = false;
            time += 1;
            if (time >= sampleTime)
            {
              this.Log(LogLevel.Noisy, "ADC State: Sampling -> SamplingDone");
              state = State.SamplingDone;
            }
            return;
          }
        case State.SamplingDone:
          {
            ready = false;
            conversionResult = GetSampleFromChannel(selectedInput);
            this.Log(LogLevel.Info, "ADC conversion completed on channel {0}: {1}", selectedInput, conversionResult);
            if (fifoEnabled)
            {
              if (fifo.Count == fifoSize)
              {
                fifoOverflowed = true;
              }
              else
              {
                ushort val = conversionResult;
                if (fifoShift)
                {
                   val = (ushort)((int)val >> 4);
                }
                if (fifoError && error)
                {
                   val |= 0x8000;
                }
                fifo.Enqueue(val);
              }

              UpdateIrqAndDreq();
            }
            if (trigger == Trigger.StartOnce)
            {
              samplingThread.Stop();
              trigger = Trigger.Nothing;
              ready = true;
              running = false;
              state = State.Waiting;
            }
            else if (trigger == Trigger.StartMany)
            {
              IterateInput();
              decimal interval = 1 + dividerIntegral + (decimal)dividerFrac / 256;
              if (interval > (decimal)sampleTime)
              {
                  state = State.Delay;
                  time = 0;
                  ready = true;
              }
              else
              {
                  state = State.Sampling;
                  time = 0;
                  ready = false;
                  error = false;
              }
            }
            return;
          }
      }
    }

    private void UpdateIrqAndDreq()
    {
      int effectiveThreshold = fifoThreshold == 0 ? 1 : (int)fifoThreshold;
      bool condition = fifo.Count >= effectiveThreshold;
      if (fifoIrqEnabled)
      {
        IRQ.Set(condition);
      }
      if (dreqEnabled)
      {
        DMARequest.Set(condition);
      }
      else
      {
        DMARequest.Set(false);
      }
    }

    private ushort GetSampleFromChannel(int channel)
    {
      if (channel < 0 || channel >= channelsCount)
      {
        return 0;
      }

      if (channel == 4)
      {
        double voltage = -0.001721 * (onboardTemperature - 27) + 0.706;
        return !temperatureSensorEnabled ? (ushort)0 : (ushort)Math.Round(((double)voltage / this.pads.PadsVoltage * ((1 << 12))));
      }

      double sample = 0;
      if (resdStream[channel] == null)
      {
        if (sampleProvider[channel].TryDequeueNewSample())
        {
          sample = (double)sampleProvider[channel].Sample.Value;
        }
        else
        {
          sample = (double)defaultSample[channel].Value;
        }
      }
      else
      {
        if (resdStream[channel].TryGetCurrentSample(this, (s) => s.Voltage, out var voltage, out _) == RESDStreamStatus.OK)
        {
          sample = (double)voltage / 1000000;
        }
      }
      ushort ret = (ushort)Math.Round(((double)sample / this.pads.PadsVoltage * ((1 << 12) - 1)));
      return ret;
    }

    private void IterateInput()
    {
      if (roundRobinChannels != 0)
      {
        for (int i = 1; i <= channelsCount; i++)
        {
          int nextChannel = (selectedInput + i) % channelsCount;
          if (BitHelper.IsBitSet(roundRobinChannels, (byte)nextChannel))
          {
            selectedInput = (byte)nextChannel;
            return;
          }
        }
      }
    }

    private void SampleChannel()
    {
    }

    private void UpdateFrequency(long frequency)
    {
      if (frequency != samplingThread.Frequency)
      {
        samplingThread.Stop();
        samplingThread.Frequency = (uint)frequency;
        Start();
      }
    }

    private void Start()
    {
      if (enabled && !running && (trigger != Trigger.Nothing))
      {
        state = State.Waiting;
        if (trigger == Trigger.StartOnce)
        {
          state = State.Sampling;
          time = 0;
          ready = false;
          error = false;
        }
        else if (trigger == Trigger.StartMany)
        {
          state = State.Sampling;
          time = 0;
          ready = false;
          error = false;
        }
        this.Log(LogLevel.Info, "Starting ADC sampling thread. Trigger: {0}", trigger);
        samplingThread.Start();
        running = true;
      }
    }

    private void DefineRegisters()
    {
      Registers.CS.Define(this)
        .WithFlag(0, valueProviderCallback: _ => enabled,
          writeCallback: (_, value) =>
          {
            enabled = value;
            if (trigger == Trigger.Nothing)
            {
              ready = true;
            }
            Start();
          },
          name: "EN")
        .WithFlag(1, valueProviderCallback: _ => temperatureSensorEnabled,
          writeCallback: (_, value) => temperatureSensorEnabled = value,
          name: "TS_EN")
        .WithFlag(2, valueProviderCallback: _ => trigger == Trigger.StartOnce,
          writeCallback: (_, value) =>
          {
            if (trigger == Trigger.StartMany)
            {
              return;
            }
            if (value)
            {
              trigger = Trigger.StartOnce;
              ready = false;
              Start();
            }
          }, name: "START_ONCE")
        .WithFlag(3, valueProviderCallback: _ => trigger == Trigger.StartMany,
          writeCallback: (_, value) =>
          {
            Trigger n = value ? Trigger.StartMany : Trigger.Nothing;
            if (trigger == n || (!value && trigger == Trigger.StartOnce))
            {
              return;
            }
            trigger = n;
            if (trigger == Trigger.Nothing)
            {
              running = false;
              ready = true;
              samplingThread.Stop();
            }
            else
            {
              Start();
            }
          },
          name: "START_MANY")
        .WithReservedBits(4, 4)
        .WithFlag(8, FieldMode.Read, valueProviderCallback: _ => ready, name: "READY")
        .WithFlag(9, FieldMode.Read, valueProviderCallback: _ => error, name: "ERR")
        .WithFlag(10, FieldMode.Read | FieldMode.WriteOneToClear, valueProviderCallback: _ => errorSticky,
          writeCallback: (_, value) => { if(value) { errorSticky = false; error = false; } }, name: "ERR_STICKY")
        .WithReservedBits(11, 1)
        .WithValueField(12, 3, valueProviderCallback: _ => selectedInput,
          writeCallback: (_, value) => {
              if (state == State.Sampling || state == State.SamplingDone) {
                  error = true;
                  errorSticky = true;
                  this.Log(LogLevel.Info, "ADC Error triggered by AINSEL write during conversion");
              }
              selectedInput = (byte)value;
          },
          name: "AINSEL")
        .WithReservedBits(15, 1)
        .WithValueField(16, 5, valueProviderCallback: _ => roundRobinChannels,
          writeCallback: (_, value) => {
              if (state == State.Sampling || state == State.SamplingDone) {
                  error = true;
                  errorSticky = true;
                  this.Log(LogLevel.Info, "ADC Error triggered by RROBIN write during conversion");
              }
              roundRobinChannels = (byte)value;
          },
          name: "RROBIN")
        .WithReservedBits(21, 11);

      Registers.RESULT.Define(this)
        .WithValueField(0, 12, FieldMode.Read,
          valueProviderCallback: _ =>
          {
            return conversionResult;
          },
          name: "RESULT");

      Registers.FCS.Define(this)
        .WithFlag(0, valueProviderCallback: _ => fifoEnabled,
          writeCallback: (_, value) => { fifoEnabled = value; UpdateIrqAndDreq(); },
          name: "EN")
        .WithFlag(1, valueProviderCallback: _ => fifoShift,
          writeCallback: (_, value) => fifoShift = value,
          name: "SHIFT")
        .WithFlag(2, valueProviderCallback: _ => fifoError,
          writeCallback: (_, value) => { fifoError = value; UpdateIrqAndDreq(); },
          name: "ERR")
        .WithFlag(3, valueProviderCallback: _ => dreqEnabled,
          writeCallback: (_, value) => { dreqEnabled = value; UpdateIrqAndDreq(); },
          name: "DREQ")
        .WithReservedBits(4, 4)
        .WithFlag(8, FieldMode.Read, valueProviderCallback: _ => fifo.Count == 0,
          name: "EMPTY")
        .WithFlag(9, FieldMode.Read, valueProviderCallback: _ => fifo.Count == fifoSize,
          name: "FULL")
        .WithFlag(10, valueProviderCallback: _ => fifoUnderflowed,
          writeCallback: (_, value) => fifoUnderflowed = value ? false : fifoUnderflowed,
          name: "UNDER")
        .WithFlag(11, valueProviderCallback: _ => fifoOverflowed,
          writeCallback: (_, value) => fifoOverflowed = value ? false : fifoOverflowed,
          name: "OVER")
        .WithReservedBits(12, 4)
        .WithValueField(16, 4, FieldMode.Read, valueProviderCallback: _ => (ulong)fifo.Count, name: "LEVEL")
        .WithReservedBits(20, 4)
        .WithValueField(24, 4, valueProviderCallback: _ => fifoThreshold,
          writeCallback: (_, value) => { fifoThreshold = (byte)value; UpdateIrqAndDreq(); },
          name: "THRESH")
        .WithReservedBits(28, 4);

      Registers.FIFO.Define(this)
        .WithValueField(0, 16, valueProviderCallback: _ =>
        {
          ushort ret = 0;
          if (fifo.TryDequeue(out ret))
          {
            UpdateIrqAndDreq();
          }
          else
          {
            fifoUnderflowed = true;
          }
          return ret;
        }, name: "VAL_AND_ERR");

      Registers.DIV.Define(this)
        .WithValueField(0, 8, valueProviderCallback: _ => dividerFrac,
          writeCallback: (_, value) =>
          {
            dividerFrac = (byte)value;
            if (value != 0)
            {
              Start();
            }
          }, name: "FRAC")
        .WithValueField(8, 16, valueProviderCallback: _ => dividerIntegral,
          writeCallback: (_, value) =>
          {
            dividerIntegral = (ushort)value;
            if (value != 0)
            {
              Start();
            }
          }, name: "INT")
        .WithReservedBits(24, 8);

      Registers.INTR.Define(this)
        .WithFlag(0, FieldMode.Read, valueProviderCallback: _ => fifo.Count >= fifoThreshold,
          name: "FIFO")
        .WithReservedBits(1, 31);

      Registers.INTE.Define(this)
        .WithFlag(0, valueProviderCallback: _ => fifoIrqEnabled,
          writeCallback: (_, value) => { fifoIrqEnabled = value; UpdateIrqAndDreq(); },
          name: "FIFO");

      Registers.INTF.Define(this)
        .WithFlag(0, valueProviderCallback: _ => fifoIrqForced,
          writeCallback: (_, value) => fifoIrqForced = value,
          name: "FIFO");

      Registers.INTS.Define(this)
        .WithFlag(0, FieldMode.Read,
          valueProviderCallback: _ => fifoIrqForced || (fifo.Count >= fifoThreshold),
          name: "FIFO");
    }

    private enum Trigger
    {
      Nothing,
      StartOnce,
      StartMany
    };

    private enum Registers
    {
      CS = 0x00,
      RESULT = 0x04,
      FCS = 0x08,
      FIFO = 0x0c,
      DIV = 0x10,
      INTR = 0x14,
      INTE = 0x18,
      INTF = 0x1c,
      INTS = 0x20
    };

    public GPIO IRQ { get; private set; }
    public GPIO DMARequest { get; }

    public bool ReadDMARequest()
    {
      return DMARequest.IsSet;
    }


    private bool dreqEnabled;
    private bool enabled;
    private bool temperatureSensorEnabled;
    private bool ready;
    private Trigger trigger;
    private bool error;
    private bool errorSticky;
    private byte selectedInput;
    private byte roundRobinChannels;
    private ushort conversionResult;

    private CircularBuffer<ushort> fifo;
    private const int fifoSize = 8;
    private bool fifoEnabled;
    private bool fifoShift;
    private bool fifoError;
    private bool fifoAssertDMA;
    private byte fifoThreshold;
    private bool fifoUnderflowed;
    private bool fifoOverflowed;

    private ushort dividerIntegral;
    private byte dividerFrac;

    private bool fifoIrqEnabled;
    private bool fifoIrqForced;

    private const int channelsCount = 5;
    private SensorSamplesFifo<ScalarSample>[] sampleProvider;
    private ScalarSample[] defaultSample;
    private readonly IManagedThread samplingThread;

    private const int sampleTime = 96; // in cycles
    private bool samplingStarted = false;

    private enum State
    {
      Waiting,
      Sampling,
      SamplingDone,
      Delay
    };
    private State state;
    private int time;
    private RP2040Pads pads;
    private bool running;
    private RESDStream<VoltageSample>[] resdStream;
    private double onboardTemperature;
  }
}
