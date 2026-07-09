using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Resets
{
    public class RP2040Resets : RP2040PeripheralBase, IKnownSize
    {
        public RP2040Resets(Machine machine, ulong address) : base(machine, address)
        {
            DefineRegisters();
            Reset();
        }

        public override void Reset()
        {
            base.Reset();
            // Default value of RESET is 0x01ffffff (all peripherals in reset)
            resetField.Value = 0x01ffffff;
            wdselField.Value = 0x0;
        }

        private void DefineRegisters()
        {
            Registers.RESET.Define(this, 0x01ffffff)
                .WithValueField(0, 32, out resetField, name: "RESET");

            Registers.WDSEL.Define(this, 0x0)
                .WithValueField(0, 32, out wdselField, name: "WDSEL");

            Registers.RESET_DONE.Define(this, 0x0)
                .WithValueField(0, 32, FieldMode.Read,
                    valueProviderCallback: _ => ~resetField.Value & 0x01ffffff,
                    name: "RESET_DONE");
        }

        private enum Registers
        {
            RESET = 0x0,
            WDSEL = 0x4,
            RESET_DONE = 0x8
        };

        IValueRegisterField resetField;
        IValueRegisterField wdselField;
    }
}
