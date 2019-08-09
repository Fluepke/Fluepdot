EESchema Schematic File Version 4
LIBS:fluepdot-cache
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 4 6
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text HLabel 1350 1350 0    50   Input ~ 10
RTS
Text HLabel 1350 1000 0    50   Input ~ 10
DTR
Text HLabel 2350 1000 2    50   Input ~ 10
GPIO0
$Comp
L fluepdot:UMH3N U4
U 1 1 5CAE7BF5
P 1850 1350
F 0 "U4" H 1850 1675 50  0000 C CNN
F 1 "UMH3N" H 1850 1584 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-363_SC-70-6" H 1850 1350 50  0001 C CNN
F 3 "" H 1850 1350 50  0001 C CNN
F 4 "UMH3NTN" H 1850 1350 50  0001 C CNN "Octopart"
	1    1850 1350
	1    0    0    -1  
$EndComp
Text HLabel 2350 1700 2    50   Input ~ 10
RTS
Wire Wire Line
	1350 1350 1450 1350
Wire Wire Line
	2250 1700 2350 1700
Wire Wire Line
	1350 1000 1450 1000
Text HLabel 2350 1350 2    50   Input ~ 10
DTR
Wire Wire Line
	2350 1350 2250 1350
Wire Wire Line
	2350 1000 2250 1000
Text HLabel 1350 1700 0    50   Input ~ 10
EN
Wire Wire Line
	1350 1700 1450 1700
Text Notes 1800 2600 0    50   ~ 10
Auto program\nDTR RTS EN IO0\n 1  1  1  1\n 0  0  1  1\n 1  0  0  1\n 0  1  1  0\n
$Comp
L power:GND #PWR?
U 1 1 5CCCFD84
P 3000 1500
F 0 "#PWR?" H 3000 1250 50  0001 C CNN
F 1 "GND" H 3005 1327 50  0000 C CNN
F 2 "" H 3000 1500 50  0001 C CNN
F 3 "" H 3000 1500 50  0001 C CNN
	1    3000 1500
	1    0    0    -1  
$EndComp
$Comp
L Device:R R11
U 1 1 5CCD0241
P 3000 1250
F 0 "R11" H 3070 1296 50  0000 L CNN
F 1 "10k" H 3070 1205 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 2930 1250 50  0001 C CNN
F 3 "~" H 3000 1250 50  0001 C CNN
F 4 "RC0805FR-0710KL" H 3000 1250 50  0001 C CNN "Octopart"
	1    3000 1250
	1    0    0    -1  
$EndComp
Text HLabel 3000 1000 1    50   Input ~ 10
GPIO2
Wire Wire Line
	3000 1100 3000 1000
Wire Wire Line
	3000 1500 3000 1400
$EndSCHEMATC
