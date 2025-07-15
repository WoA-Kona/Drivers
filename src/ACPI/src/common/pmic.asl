Device (PMIC)
{
    Name (_HID, "KONA1003")
    Name (_DEP,
        Package(0x1) {
            \_SB_.SPMI
        }
    )

    Method (PCFG)
    {
        Return(Package()
        {
            6, // Number of PMICs
            Package() // PM8250
            {
                0x0,
                0x1
            },
            Package() // PM8150B
            {
                0x2,
                0x3
            },
            Package() // PM8150L/A
            {
                0x4,
                0x5
            },
            Package() // PMK8002
            {
                0x6,
                0x7
            },
            Package() // PMX55
            {
                0x8,
                0x9
            },
            Package() // PM8009
            {
                0xA,
                0xB
            }
        })
    }
}
