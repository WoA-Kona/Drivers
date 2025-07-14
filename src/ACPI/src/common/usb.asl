Device(URS0)
{
    Name (_HID, "QCOM0497")
    Name(_CID, "PNP0CA1")
    Name (_UID, 0)
    Name (_CCA, 0)

    Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x0A600000, 0x000FFFFF)
    })

    Device(USB0)
    {
        Name(_ADR, 0)

        Name(_CRS, ResourceTemplate() {
            Interrupt(ResourceConsumer, Level, ActiveHigh, Shared, , , ) {165}
            Interrupt(ResourceConsumer, Level, ActiveHigh, Shared, , , ) {163}
            Interrupt(ResourceConsumer, Level, ActiveHigh, SharedAndWake, , , ) {529}
            Interrupt(ResourceConsumer, Edge, ActiveHigh, SharedAndWake, , , ) {527}
            Interrupt(ResourceConsumer, Edge, ActiveHigh, SharedAndWake, , , ) {526}
        })

        Device(RHUB) {
            Name( _ADR, 0x00000000)

            Device(PRT1) {
                Name(_ADR, 0x00000001)

                Name(_UPC, Package()
                {
                    0x01,                       // Port is connectable.
                    0x09,                       // Connector type: Type C connector - USB2 and SS with switch.
                    0x00000000,                 // Reserved0 - must be zero.
                    0x00000000                  // Reserved1 - must be zero.
                })

                Name(_PLD, Package()
                {
                    Buffer()
                    {
                        0x82,                   // Revision 2, ignore color.
                        0x00,0x00,0x00,         // Color (ignored).
                        0x00,0x00,0x00,0x00,    // Width and height.
                        0x69,                   // User visible; Back panel; VerticalPos:Center.
                        0x0c,                   // HorizontalPos:0; Shape:Vertical Rectangle; GroupOrientation:0.
                        0x00,0x00,              // Group Token:0; Group Position:0; So Connector ID is 0.
                        0x00,0x00,0x00,0x00,    // Not ejectable.
                        0xFF,0xFF,0xFF,0xFF     // Vert. and horiz. offsets not supplied.
                    }
                })
            }
        }

        Method (_DSM, 0x4, NotSerialized)
        {
            switch(ToBuffer(Arg0)) {
                // UFX interface identifier
                case(ToUUID("FE56CFEB-49D5-4378-A8A2-2978DBE54AD2")) {
                    // Function selector
                    switch(Arg2) {
                        // Function 1: Return number of supported USB PHYSICAL endpoints
                        case(1) {
                            Return(32);
                        }
                        default {
                            Return (Buffer(){ 0x0 });
                        }
                    }
                }
                default {
                    Return (Buffer(){ 0x0 });
                }
            }
        }
    }

    Device(UFN0)
    {
        Name(_ADR, 1)

        Name(_CRS, ResourceTemplate() {
            Interrupt(ResourceConsumer, Level, ActiveHigh, Shared, , , ) {165}
            Interrupt(ResourceConsumer, Level, ActiveHigh, Shared, , , ) {163}
        })

        Device( RHUB) {
            Name( _ADR, 0)

            Device(PRT1) {
                Name(_ADR, 1)

                Name(_UPC, Package()
                {
                    0x01,                       // Port is connectable.
                    0x09,                       // Connector type: Type C connector - USB2 and SS with switch.
                    0x00000000,                 // Reserved0 - must be zero.
                    0x00000000                  // Reserved1 - must be zero.
                })

                Name(_PLD, Package()
                {
                    Buffer()
                    {
                        0x82,                   // Revision 2, ignore color.
                        0x00,0x00,0x00,         // Color (ignored).
                        0x00,0x00,0x00,0x00,    // Width and height.
                        0x69,                   // User visible; Back panel; VerticalPos:Center.
                        0x0c,                   // HorizontalPos:0; Shape:Vertical Rectangle; GroupOrientation:0.
                        0x00,0x00,              // Group Token:0; Group Position:0; So Connector ID is 0.
                        0x00,0x00,0x00,0x00,    // Not ejectable.
                        0xFF,0xFF,0xFF,0xFF     // Vert. and horiz. offsets not supplied.
                    }
                })
            }
        }

        Method (_DSM, 0x4, NotSerialized)
        {
            switch(ToBuffer(Arg0)) {
                // UFX interface identifier
                case(ToUUID("FE56CFEB-49D5-4378-A8A2-2978DBE54AD2")) {
                    // Function selector
                    switch(Arg2) {
                        // Function 1: Return number of supported USB PHYSICAL endpoints
                        case(1) {
                            Return(32);
                        }
                        default {
                            Return (Buffer(){ 0x0 });
                        }
                    }
                }
                default {
                    Return (Buffer(){ 0x0 });
                }
            }
        }
    }
}