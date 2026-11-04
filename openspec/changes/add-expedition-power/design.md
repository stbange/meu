# M1a design
Integer kJ and heat units update once per 60 fixed ticks. The daylight phase is a deterministic ten-active-minute triangular cycle. Idle demand is 1 kJ/s, movement adds 24; fixed panels yield up to 18 and deployed parked collectors up to 150. Main capacity is 30000; reserve planning stops travel below 4000. A separate 2000 kJ life-support allocation cannot be spent by ordinary systems. Heat rejects costly operations at 10000 and trips grid input at 9000. Cooling priority trades 35 units/s cooling against 10 in fabrication mode.

The tower socket occupies robot X +/-15, Z -120..-175, reachable on the existing supported causeway. The 12-second visible grid cycle has a three-second surge. Connection requires a parked unit and workshop isolation. A pulse trips the input buffer without disabling doors or the lift. Future repair/technology steps extend this state.

Station overlays retain the surrounding first-person room and clear movement ownership. All commands use the C simulation on native and web. Saves use schema 2 with explicit extension fields and CRC; schema 1 imports initialize the new systems safely. No global tool settings or sibling project dependencies.
