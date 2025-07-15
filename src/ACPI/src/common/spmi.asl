Device (SPMI)
{
    Name (_HID, "KONA1002")

    Method(_CRS, 0x0, NotSerialized) {
        Name (RBUF, ResourceTemplate ()
        {
            Memory32Fixed (ReadWrite, 0x0C440000, 0x00001100) // CORE
            Memory32Fixed (ReadWrite, 0x0C600000, 0x02000000) // CHNLS
            Memory32Fixed (ReadWrite, 0x0E600000, 0x00100000) // OBSRVR
            Memory32Fixed (ReadWrite, 0x0E700000, 0x000A0000) // INTR
            Memory32Fixed (ReadWrite, 0x0C40A000, 0x00026000) // CNFG
        })
        Return (RBUF)
    }
}