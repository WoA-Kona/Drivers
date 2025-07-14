DefinitionBlock("DSDT.AML", "DSDT", 0x02, "KONA  ", "KONA8250", 0)
{
    Scope(\_SB_) {
        Include("cpu.asl")

        Include("ufs.asl")
        Include("usb.asl")
        Include("gpio.asl")
   }
}