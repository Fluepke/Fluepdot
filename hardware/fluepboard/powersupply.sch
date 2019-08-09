EESchema Schematic File Version 4
LIBS:fluepdot-cache
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 6
Title "Powersupply"
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text HLabel 1000 1250 0    50   Input ~ 10
V_in
Text HLabel 8250 1250 2    50   Output ~ 10
+3V3
$Comp
L Regulator_Switching:MCP16301 U2
U 1 1 5CCE859B
P 5300 1350
F 0 "U2" H 5300 1717 50  0000 C CNN
F 1 "MCP16301" H 5300 1626 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-6" H 5350 1000 50  0001 L CNN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/20005004D.pdf" H 5000 1700 50  0001 C CNN
F 4 "MCP16301T-I/CHY" H 5300 1350 50  0001 C CNN "Octopart"
	1    5300 1350
	1    0    0    -1  
$EndComp
Text HLabel 3700 1250 0    50   Input ~ 10
+12V
$Comp
L Device:C C4
U 1 1 5CCEA078
P 4400 1600
F 0 "C4" H 4515 1646 50  0000 L CNN
F 1 "10uF" H 4515 1555 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 4438 1450 50  0001 C CNN
F 3 "~" H 4400 1600 50  0001 C CNN
F 4 "FEHLER!!!" H 4400 1600 50  0001 C CNN "Octopart"
	1    4400 1600
	1    0    0    -1  
$EndComp
$Comp
L Device:R R4
U 1 1 5CCEB42E
P 4800 1550
F 0 "R4" H 4730 1504 50  0000 R CNN
F 1 "10k" H 4730 1595 50  0000 R CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 4730 1550 50  0001 C CNN
F 3 "~" H 4800 1550 50  0001 C CNN
F 4 "RC0805FR-0710KL" H 4800 1550 50  0001 C CNN "Octopart"
	1    4800 1550
	-1   0    0    1   
$EndComp
Wire Wire Line
	4800 1350 4900 1350
$Comp
L Device:C C5
U 1 1 5CCECC30
P 6200 1500
F 0 "C5" H 6315 1546 50  0000 L CNN
F 1 "100nF" H 6315 1455 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 6238 1350 50  0001 C CNN
F 3 "~" H 6200 1500 50  0001 C CNN
F 4 "C0805C104Z5VACTU" H 6200 1500 50  0001 C CNN "Octopart"
	1    6200 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	6200 1250 6200 1350
Wire Wire Line
	6200 1250 6300 1250
Connection ~ 6200 1250
$Comp
L Diode:B140-E3 D2
U 1 1 5CCEEB60
P 6200 2000
F 0 "D2" V 6154 2079 50  0000 L CNN
F 1 "B0540WS-7" V 6245 2079 50  0000 L CNN
F 2 "Diode_SMD:D_SOD-323" H 6200 1825 50  0001 C CNN
F 3 "" H 6200 2000 50  0001 C CNN
F 4 "B0540WS-7" H 6200 2000 50  0001 C CNN "Octopart"
	1    6200 2000
	0    1    1    0   
$EndComp
$Comp
L Device:L L1
U 1 1 5CCEEF58
P 6450 1750
F 0 "L1" V 6640 1750 50  0000 C CNN
F 1 "15uH" V 6549 1750 50  0000 C CNN
F 2 "fluepdot:744025150" H 6450 1750 50  0001 C CNN
F 3 "~" H 6450 1750 50  0001 C CNN
F 4 "744025150" H 6450 1750 50  0001 C CNN "Octopart"
	1    6450 1750
	0    -1   -1   0   
$EndComp
$Comp
L Device:C C6
U 1 1 5CCF11CB
P 7300 1500
F 0 "C6" H 7415 1546 50  0000 L CNN
F 1 "10uF" H 7415 1455 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 7338 1350 50  0001 C CNN
F 3 "~" H 7300 1500 50  0001 C CNN
F 4 "FEHLER!!!" H 7300 1500 50  0001 C CNN "Octopart"
	1    7300 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	7300 1250 7300 1350
Wire Wire Line
	7300 1250 7750 1250
$Comp
L Device:R R5
U 1 1 5CCF201F
P 6950 1500
F 0 "R5" H 7020 1546 50  0000 L CNN
F 1 "31k6" H 7020 1455 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 6880 1500 50  0001 C CNN
F 3 "~" H 6950 1500 50  0001 C CNN
F 4 "RC0805FR-07316KL" H 6950 1500 50  0001 C CNN "Octopart"
	1    6950 1500
	1    0    0    -1  
$EndComp
$Comp
L Device:R R6
U 1 1 5CCF2712
P 6950 1900
F 0 "R6" H 7020 1946 50  0000 L CNN
F 1 "10k" H 7020 1855 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 6880 1900 50  0001 C CNN
F 3 "~" H 6950 1900 50  0001 C CNN
F 4 "RC0805FR-0710KL" H 6950 1900 50  0001 C CNN "Octopart"
	1    6950 1900
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5CCF2A2C
P 7300 2300
F 0 "#PWR?" H 7300 2050 50  0001 C CNN
F 1 "GND" H 7305 2127 50  0000 C CNN
F 2 "" H 7300 2300 50  0001 C CNN
F 3 "" H 7300 2300 50  0001 C CNN
	1    7300 2300
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5CCF2ECD
P 6950 2300
F 0 "#PWR?" H 6950 2050 50  0001 C CNN
F 1 "GND" H 6955 2127 50  0000 C CNN
F 2 "" H 6950 2300 50  0001 C CNN
F 3 "" H 6950 2300 50  0001 C CNN
	1    6950 2300
	1    0    0    -1  
$EndComp
Wire Wire Line
	6950 2050 6950 2300
Wire Wire Line
	7300 1650 7300 2300
Wire Wire Line
	6200 1650 6200 1750
Wire Wire Line
	6200 1750 6300 1750
Wire Wire Line
	5700 1550 5800 1550
Wire Wire Line
	6200 1750 6050 1750
Connection ~ 6200 1750
Wire Wire Line
	6200 1750 6200 1850
$Comp
L power:GND #PWR?
U 1 1 5CCF9F43
P 6200 2300
F 0 "#PWR?" H 6200 2050 50  0001 C CNN
F 1 "GND" H 6205 2127 50  0000 C CNN
F 2 "" H 6200 2300 50  0001 C CNN
F 3 "" H 6200 2300 50  0001 C CNN
	1    6200 2300
	1    0    0    -1  
$EndComp
Wire Wire Line
	6200 2150 6200 2300
$Comp
L Diode:1N4148 D3
U 1 1 5CCFB91D
P 6450 1250
F 0 "D3" H 6450 1466 50  0000 C CNN
F 1 "1N4148" H 6450 1375 50  0000 C CNN
F 2 "Diode_SMD:D_SOD-323" H 6450 1075 50  0001 C CNN
F 3 "https://assets.nexperia.com/documents/data-sheet/1N4148_1N4448.pdf" H 6450 1250 50  0001 C CNN
F 4 "1N4148WS-7-F" H 6450 1250 50  0001 C CNN "Octopart"
	1    6450 1250
	1    0    0    -1  
$EndComp
Wire Wire Line
	6600 1250 6700 1250
Wire Wire Line
	6950 1650 6950 1700
Wire Wire Line
	6950 1350 6950 1250
Wire Wire Line
	6600 1750 6700 1750
Wire Wire Line
	6700 1750 6700 1250
Connection ~ 6700 1250
Wire Wire Line
	6700 1250 6950 1250
Wire Wire Line
	6950 1250 7300 1250
Connection ~ 6950 1250
Connection ~ 7300 1250
Wire Wire Line
	5700 1250 6200 1250
Wire Wire Line
	5700 1350 6050 1350
Text GLabel 7000 1700 2    50   Input ~ 10
Vfb
Wire Wire Line
	6950 1700 7000 1700
Connection ~ 6950 1700
Wire Wire Line
	6950 1700 6950 1750
Text GLabel 5800 1550 2    50   Input ~ 10
Vfb
$Comp
L power:GND #PWR?
U 1 1 5CD0F14B
P 5300 2300
F 0 "#PWR?" H 5300 2050 50  0001 C CNN
F 1 "GND" H 5305 2127 50  0000 C CNN
F 2 "" H 5300 2300 50  0001 C CNN
F 3 "" H 5300 2300 50  0001 C CNN
	1    5300 2300
	1    0    0    -1  
$EndComp
Wire Wire Line
	5300 1750 5300 2300
$Comp
L power:GND #PWR?
U 1 1 5CD0FCF1
P 4400 2300
F 0 "#PWR?" H 4400 2050 50  0001 C CNN
F 1 "GND" H 4405 2127 50  0000 C CNN
F 2 "" H 4400 2300 50  0001 C CNN
F 3 "" H 4400 2300 50  0001 C CNN
	1    4400 2300
	1    0    0    -1  
$EndComp
Wire Wire Line
	4400 1750 4400 2300
Wire Wire Line
	4400 1450 4400 1250
Wire Wire Line
	3700 1250 3950 1250
Connection ~ 4400 1250
Text Notes 3450 2650 0    50   ~ 10
http://ww1.microchip.com/downloads/en/devicedoc/20005004d.pdf Figure 5.1
$Comp
L Device:C C3
U 1 1 5CC08FC1
P 3950 1600
F 0 "C3" H 4065 1646 50  0000 L CNN
F 1 "10uF" H 4065 1555 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 3988 1450 50  0001 C CNN
F 3 "~" H 3950 1600 50  0001 C CNN
F 4 "FEHLER!!!" H 3950 1600 50  0001 C CNN "Octopart"
	1    3950 1600
	1    0    0    -1  
$EndComp
Wire Wire Line
	3950 1450 3950 1250
$Comp
L power:GND #PWR?
U 1 1 5CC0A124
P 3950 2300
F 0 "#PWR?" H 3950 2050 50  0001 C CNN
F 1 "GND" H 3955 2127 50  0000 C CNN
F 2 "" H 3950 2300 50  0001 C CNN
F 3 "" H 3950 2300 50  0001 C CNN
	1    3950 2300
	1    0    0    -1  
$EndComp
Wire Wire Line
	3950 2300 3950 1750
$Comp
L Device:C C7
U 1 1 5CC0C216
P 7750 1500
F 0 "C7" H 7865 1546 50  0000 L CNN
F 1 "10uF" H 7865 1455 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 7788 1350 50  0001 C CNN
F 3 "~" H 7750 1500 50  0001 C CNN
F 4 "FEHLER!!!" H 7750 1500 50  0001 C CNN "Octopart"
	1    7750 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	7750 1350 7750 1250
$Comp
L power:GND #PWR?
U 1 1 5CC0D6E9
P 7750 2300
F 0 "#PWR?" H 7750 2050 50  0001 C CNN
F 1 "GND" H 7755 2127 50  0000 C CNN
F 2 "" H 7750 2300 50  0001 C CNN
F 3 "" H 7750 2300 50  0001 C CNN
	1    7750 2300
	1    0    0    -1  
$EndComp
Wire Wire Line
	7750 2300 7750 1650
Connection ~ 7750 1250
Wire Wire Line
	6050 1750 6050 1350
Wire Wire Line
	4400 1250 4650 1250
Wire Wire Line
	4800 1700 4800 1850
$Comp
L Diode:B140-E3 D8
U 1 1 5CF3445A
P 1550 1600
F 0 "D8" V 1504 1679 50  0000 L CNN
F 1 "B140-E3" V 1595 1679 50  0000 L CNN
F 2 "Diode_SMD:D_SMA" H 1550 1425 50  0001 C CNN
F 3 "http://www.vishay.com/docs/88946/b120.pdf" H 1550 1600 50  0001 C CNN
F 4 "B140-E3/61T" H 1550 1600 50  0001 C CNN "Octopart"
	1    1550 1600
	0    1    1    0   
$EndComp
Wire Wire Line
	1550 1450 1550 1250
$Comp
L power:GND #PWR?
U 1 1 5CF37C1A
P 1550 1850
F 0 "#PWR?" H 1550 1600 50  0001 C CNN
F 1 "GND" H 1555 1677 50  0000 C CNN
F 2 "" H 1550 1850 50  0001 C CNN
F 3 "" H 1550 1850 50  0001 C CNN
	1    1550 1850
	1    0    0    -1  
$EndComp
Wire Wire Line
	1550 1850 1550 1750
Wire Wire Line
	7750 1250 8250 1250
$Comp
L Device:Fuse F1
U 1 1 5CFE25B2
P 1300 1250
F 0 "F1" V 1103 1250 50  0000 C CNN
F 1 "Fuse" V 1194 1250 50  0000 C CNN
F 2 "Fuse:Fuse_1206_3216Metric" V 1230 1250 50  0001 C CNN
F 3 "~" H 1300 1250 50  0001 C CNN
F 4 "SF-1206F400-2" H 1300 1250 50  0001 C CNN "Octopart"
	1    1300 1250
	0    1    1    0   
$EndComp
Wire Wire Line
	1000 1250 1150 1250
Wire Wire Line
	1450 1250 1550 1250
Connection ~ 1550 1250
Text HLabel 2450 1250 2    50   Input ~ 10
+12V
Wire Wire Line
	1550 1250 2200 1250
$Comp
L Device:CP C12
U 1 1 5D1CDD84
P 2200 1600
F 0 "C12" H 2318 1646 50  0000 L CNN
F 1 "330uF" H 2318 1555 50  0000 L CNN
F 2 "Capacitor_SMD:CP_Elec_6.3x7.7" H 2238 1450 50  0001 C CNN
F 3 "~" H 2200 1600 50  0001 C CNN
F 4 "UCV1E331MCL1GS" H 2200 1600 50  0001 C CNN "Octopart"
	1    2200 1600
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5D1CE006
P 2200 1900
F 0 "#PWR?" H 2200 1650 50  0001 C CNN
F 1 "GND" H 2205 1727 50  0000 C CNN
F 2 "" H 2200 1900 50  0001 C CNN
F 3 "" H 2200 1900 50  0001 C CNN
	1    2200 1900
	1    0    0    -1  
$EndComp
Wire Wire Line
	2200 1900 2200 1750
Wire Wire Line
	2200 1450 2200 1250
Connection ~ 2200 1250
Wire Wire Line
	2200 1250 2450 1250
Wire Wire Line
	3700 1000 4050 1000
Wire Wire Line
	4400 1000 4400 1250
Wire Wire Line
	4350 1000 4400 1000
Text HLabel 3700 1000 0    50   Input ~ 10
VUSB
$Comp
L Diode:B140-E3 D1
U 1 1 5CFD2FEC
P 4200 1000
F 0 "D1" V 4154 1079 50  0000 L CNN
F 1 "B140-E3" V 4245 1079 50  0000 L CNN
F 2 "Diode_SMD:D_SMA" H 4200 825 50  0001 C CNN
F 3 "http://www.vishay.com/docs/88946/b120.pdf" H 4200 1000 50  0001 C CNN
F 4 "B140-E3/61T" H 4200 1000 50  0001 C CNN "Octopart"
	1    4200 1000
	-1   0    0    1   
$EndComp
Wire Wire Line
	4800 1350 4800 1400
Wire Wire Line
	4800 1850 4650 1850
Wire Wire Line
	4650 1850 4650 1250
Connection ~ 4650 1250
Wire Wire Line
	4650 1250 4900 1250
$Comp
L Diode:B140-E3 D?
U 1 1 5D483AD4
P 4200 1250
F 0 "D?" V 4154 1329 50  0000 L CNN
F 1 "B140-E3" V 4245 1329 50  0000 L CNN
F 2 "Diode_SMD:D_SMA" H 4200 1075 50  0001 C CNN
F 3 "http://www.vishay.com/docs/88946/b120.pdf" H 4200 1250 50  0001 C CNN
F 4 "B140-E3/61T" H 4200 1250 50  0001 C CNN "Octopart"
	1    4200 1250
	-1   0    0    1   
$EndComp
Wire Wire Line
	3950 1250 4050 1250
Connection ~ 3950 1250
Wire Wire Line
	4350 1250 4400 1250
$EndSCHEMATC
