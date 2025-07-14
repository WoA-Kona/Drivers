Device (GPIO)
{
    Name (_HID, "KONA1001")

    Method(_CRS, 0x0, NotSerialized) {
        Name (RBUF, ResourceTemplate ()
        {
            Memory32Fixed (ReadWrite, 0xF100000, 0xB00000)
            Interrupt(ResourceConsumer, Level, ActiveHigh, Shared, , , ) {240}
            Interrupt(ResourceConsumer, Level, ActiveHigh, Shared, , , ) {240}
            Interrupt(ResourceConsumer, Level, ActiveHigh, Shared, , , ) {240}
        })
        Return (RBUF)
    }
}