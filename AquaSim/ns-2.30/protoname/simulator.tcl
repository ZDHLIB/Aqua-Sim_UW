set opt(chan)       Channel/UnderwaterChannel
set opt(prop)       Propagation/UnderwaterPropagation
set opt(netif)      Phy/UnderwaterPhy
set opt(mac)        Mac/UnderwaterMac/TMac
set opt(ifq)        Queue/DropTail/PriQueue
set opt(ll)     LL
set opt(energy)         EnergyModel
set opt(txpower)        2.0
set opt(rxpower)        0.75
set opt(initialenergy)  10000
set opt(idlepower)      0.008
set opt(ant)            Antenna/OmniAntenna
set opt(minspeed)           1  ;#minimum speed of node
set opt(maxspeed)           6   ;#maximum speed of node
set opt(speed)              5.5  ;#speed of node
set opt(position_update_interval) 0.3  ;# the length of period to update node's position
set opt(packet_size) 50  ;#50 bytes
set opt(routing_control_packet_size) 20 ;#bytes 
set opt(ifqlen)     50  ;# max queue length in if
set opt(nn)         10   ;# number of nodes
set opt(dz)         10
set opt(layers)     1
set opt(x)          100 ;# X dimension of the topography
set opt(y)          100  ;# Y dimension of the topography
set opt(z)          100
set opt(seed)       348.88
set opt(stop)       500 ;# simulation time
set opt(traceFile)     "protoname.tr"  ;# trace file
set opt(datafile)   "protoname.data"
set opt(nam)            "protoname.nam"  ;# nam file
set opt(routing)    DBR
set opt(width)           100
set opt(interval)        0.3
set opt(range)           120    ;#range of each node in meters

#Bind for mac 
set opt(bit_rate)                     1.0e4
set opt(encoding_efficiency)          1
set opt(ND_window)                    1
set opt(ACKND_window)                 1
set opt(PhaseOne_window)              3
set opt(PhaseTwo_window)              1
set opt(PhaseTwo_interval)            0.5
set opt(IntervalPhase2Phase3)         1
set opt(duration)                     0.1
set opt(PhyOverhead)                  8
set opt(large_packet_size)            480 ;# 60 bytes
set opt(short_packet_size)            40  ;# 5 bytes
set opt(PhaseOne_cycle)               4 ;
set opt(PhaseTwo_cycle)               2 ;
set opt(PeriodInterval)               1
set opt(transmission_time_error)      0.0001;
set opt(ContentionWindow)             0.1;


puts "The data file is $opt(datafile)"
puts "The trace file is $opt(datafile)"

LL set mindelay_        50us
LL set delay_           25us
LL set bandwidth_       0   ;# not used

Mac/UnderwaterMac      set bit_rate_                    $opt(bit_rate)
Mac/UnderwaterMac      set encoding_efficiency_         $opt(encoding_efficiency)
Mac/UnderwaterMac/TMac set ND_window_                   $opt(ND_window)
Mac/UnderwaterMac/TMac set ACKND_window_                $opt(ACKND_window)
Mac/UnderwaterMac/TMac set PhaseOne_window_             $opt(PhaseOne_window)
Mac/UnderwaterMac/TMac set PhaseTwo_window_             $opt(PhaseTwo_window)
Mac/UnderwaterMac/TMac set IntervalPhase2Phase3_        $opt(IntervalPhase2Phase3)
Mac/UnderwaterMac/TMac set duration_                    $opt(duration)
Mac/UnderwaterMac/TMac set PhyOverhead_                 $opt(PhyOverhead)
Mac/UnderwaterMac/TMac set large_packet_size_           $opt(large_packet_size)
Mac/UnderwaterMac/TMac set short_packet_size_           $opt(short_packet_size)
Mac/UnderwaterMac/TMac set PhaseOne_cycle_              $opt(PhaseOne_cycle)
Mac/UnderwaterMac/TMac set PeriodInterval_              $opt(PeriodInterval)
Mac/UnderwaterMac/TMac set transmission_time_error_     $opt(transmission_time_error)
Mac/UnderwaterMac/TMac set ContentionWindow_            $opt(ContentionWindow)
Mac/UnderwaterMac/TMac set TransmissionRange_           90

Phy/UnderwaterPhy set CPThresh_  10  ;#10.0
Phy/UnderwaterPhy set CSThresh_  0    ;#1.559e-11
Phy/UnderwaterPhy set RXThresh_  0    ;#3.652e-10
Phy/UnderwaterPhy set Pt_  0.2818
Phy/UnderwaterPhy set freq_  25  ;# 25khz  
Phy/UnderwaterPhy set K_ 2.0    ;# spherical spreading

# ==================================================================
# Main Program
# =================================================================

set ns_ [new Simulator]
set topo  [new Topography]

$topo load_cubicgrid $opt(x) $opt(y) $opt(z)

set tracefd [open $opt(traceFile) w]
$ns_ trace-all $tracefd

set nf [open $opt(nam) w]
$ns_ namtrace-all-wireless $nf $opt(x) $opt(y)

set data [open $opt(datafile) a]

set total_number  [expr $opt(nn)-1]
set god_ [create-god  $opt(nn)]

$ns_ at 0.0 "$god_  set_filename $opt(datafile)"

set chan_1_ [new $opt(chan)]

global defaultRNG
$defaultRNG seed $opt(seed)

$ns_ node-config -adhocRouting $opt(routing) \
         -llType $opt(ll) \
         -macType $opt(mac) \
         -ifqType $opt(ifq) \
         -ifqLen $opt(ifqlen) \
         -antType $opt(ant) \
         -propType $opt(prop) \
         -phyType $opt(netif) \
         -agentTrace ON \
         -routerTrace ON \
         -macTrace ON\
         -topoInstance $topo\
         -energyModel $opt(energy)\
         -txPower $opt(txpower)\
         -rxPower $opt(rxpower)\
         -initialEnergy $opt(initialenergy)\
         -idlePower $opt(idlepower)\
         -channel $chan_1_


for {set i 0} {$i < $opt(nn)} {incr i} {
    set node_($i) [$ns_ node]
    $god_ new_node $node_(0)
    $node_($i) set X_ [ expr {$opt(x) * rand()} ]
    $node_($i) set Y_ [ expr {$opt(y) * rand()} ]
    $node_($i) set Y_ [ expr {$opt(y) * rand()} ]
    $node_($i) set max_speed $opt(maxspeed)
    $node_($i) set min_speed $opt(minspeed)
    $node_($i) set position_update_interval_ $opt(position_update_interval)
    $node_($i) move
}

#$ns_ initial_node_pos node size. 
#This command defines the node initial position in nam. <size> denotes the size of node in nam
$node_(0) label "Sink"
set a_(0) [new Agent/UWSink]
$ns_ attach-agent $node_(0) $a_(0)
$ns_ initial_node_pos $node_(0) 7
for {set i 1} {$i < $opt(nn)} { incr i } {
    $ns_ initial_node_pos $node_($i) 4
    set a_($i) [new Agent/UWSink]
    $a_(1) cmd set-range $opt(range)
    $a_(1) cmd set-target-x -20
    $a_(1) cmd set-target-y -10
    $a_(1) cmd set-target-z -20
    $a_(1) cmd set-filename $opt(datafile)
    $a_(1) cmd set-packetsize $opt(packet_size)
    $ns_ attach-agent $node_($i) $a_($i)
}

$ns_ at 25 "$ns_ nam-end-wireless 15.0"
$ns_ at 25 "stop"


$ns_ at 25.0004 "$ns_ halt"
$ns_ at 25.0002 "puts \"Ending Simulation...\" "



proc stop {} {
    global ns_ tracefd nf opt
    $ns_ flush-trace
    close $tracefd
    close $nf
    exec nam $opt(nam) &
}
$ns_ run















