EESchema Schematic File Version 4
LIBS:reverse-cache
EELAYER 29 0
EELAYER END
$Descr A3 16535 11693
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Regulator_Linear:L7812 U?
U 1 1 5D06EB2E
P 4650 8000
F 0 "U?" V 4604 8105 50  0000 L CNN
F 1 "L7812" V 4695 8105 50  0000 L CNN
F 2 "" H 4675 7850 50  0001 L CIN
F 3 "http://www.st.com/content/ccc/resource/technical/document/datasheet/41/4f/b3/b0/12/d4/47/88/CD00000444.pdf/files/CD00000444.pdf/jcr:content/translations/en.CD00000444.pdf" H 4650 7950 50  0001 C CNN
	1    4650 8000
	0    1    1    0   
$EndComp
$Comp
L Connector:Conn_01x01_Male J?
U 1 1 5D0700BB
P 3800 8050
F 0 "J?" V 3954 7962 50  0000 R CNN
F 1 "Conn_01x01_Male" V 3750 8400 50  0000 R CNN
F 2 "" H 3800 8050 50  0001 C CNN
F 3 "~" H 3800 8050 50  0001 C CNN
	1    3800 8050
	0    -1   -1   0   
$EndComp
$Comp
L Connector_Generic:Conn_01x04 J?
U 1 1 5D07165C
P 3300 7900
F 0 "J?" H 3380 7892 50  0000 L CNN
F 1 "Conn_01x04" H 3380 7801 50  0000 L CNN
F 2 "" H 3300 7900 50  0001 C CNN
F 3 "~" H 3300 7900 50  0001 C CNN
	1    3300 7900
	1    0    0    -1  
$EndComp
$Comp
L dk_Logic-Flip-Flops:CD4013BE U?
U 1 1 5D072C1E
P 3150 6700
F 0 "U?" V 3153 5756 60  0000 R CNN
F 1 "CD4013BE" V 3047 5756 60  0000 R CNN
F 2 "digikey-footprints:DIP-14_W3mm" H 3350 6900 60  0001 L CNN
F 3 "http://www.ti.com/lit/ds/symlink/cd4013b.pdf" H 3350 7000 60  0001 L CNN
F 4 "296-2033-5-ND" H 3350 7100 60  0001 L CNN "Digi-Key_PN"
F 5 "CD4013BE" H 3350 7200 60  0001 L CNN "MPN"
F 6 "Integrated Circuits (ICs)" H 3350 7300 60  0001 L CNN "Category"
F 7 "Logic - Flip Flops" H 3350 7400 60  0001 L CNN "Family"
F 8 "http://www.ti.com/lit/ds/symlink/cd4013b.pdf" H 3350 7500 60  0001 L CNN "DK_Datasheet_Link"
F 9 "/product-detail/en/texas-instruments/CD4013BE/296-2033-5-ND/67245" H 3350 7600 60  0001 L CNN "DK_Detail_Page"
F 10 "IC FF D-TYPE DUAL 1BIT 14DIP" H 3350 7700 60  0001 L CNN "Description"
F 11 "Texas Instruments" H 3350 7800 60  0001 L CNN "Manufacturer"
F 12 "Active" H 3350 7900 60  0001 L CNN "Status"
	1    3150 6700
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R?
U 1 1 5D07D13D
P 5900 8300
F 0 "R?" V 5800 8300 50  0000 C CNN
F 1 "220k" V 5900 8300 50  0000 C CNN
F 2 "" V 5830 8300 50  0001 C CNN
F 3 "~" H 5900 8300 50  0001 C CNN
	1    5900 8300
	0    1    1    0   
$EndComp
$Comp
L Device:R R?
U 1 1 5D07EB23
P 5900 8050
F 0 "R?" V 5800 8050 50  0000 C CNN
F 1 "220k" V 5900 8050 50  0000 C CNN
F 2 "" V 5830 8050 50  0001 C CNN
F 3 "~" H 5900 8050 50  0001 C CNN
	1    5900 8050
	0    1    1    0   
$EndComp
$Comp
L Device:R R?
U 1 1 5D07F3C6
P 5900 7850
F 0 "R?" V 5800 7850 50  0000 C CNN
F 1 "220k" V 5900 7850 50  0000 C CNN
F 2 "" V 5830 7850 50  0001 C CNN
F 3 "~" H 5900 7850 50  0001 C CNN
	1    5900 7850
	0    1    1    0   
$EndComp
$Comp
L Device:R R?
U 1 1 5D07F7F0
P 5200 7850
F 0 "R?" V 5100 7850 50  0000 C CNN
F 1 "220k" V 5200 7850 50  0000 C CNN
F 2 "" V 5130 7850 50  0001 C CNN
F 3 "~" H 5200 7850 50  0001 C CNN
	1    5200 7850
	0    1    1    0   
$EndComp
Wire Wire Line
	5400 7850 5350 7850
Wire Wire Line
	6100 7850 6050 7850
Wire Wire Line
	3450 7100 3450 7350
Wire Wire Line
	3450 7350 3800 7350
Wire Wire Line
	3800 7350 3800 7800
$Comp
L reverse:MC14015BCP U?
U 1 1 5D0887C3
P 3550 5500
F 0 "U?" V 3479 5938 50  0000 L CNN
F 1 "MC14015BCP" V 3570 5938 50  0000 L CNN
F 2 "" H 3550 5500 50  0001 C CNN
F 3 "" H 3550 5500 50  0001 C CNN
	1    3550 5500
	0    1    1    0   
$EndComp
Wire Wire Line
	3800 5850 3800 6750
Wire Wire Line
	3800 6750 3450 6750
Wire Wire Line
	3450 6750 3450 7100
Connection ~ 3450 7100
$Comp
L Diode:1N4148 D?
U 1 1 5D08CAE2
P 2600 6800
F 0 "D?" V 2554 6879 50  0000 L CNN
F 1 "1N4148" V 2645 6879 50  0000 L CNN
F 2 "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal" H 2600 6625 50  0001 C CNN
F 3 "https://assets.nexperia.com/documents/data-sheet/1N4148_1N4448.pdf" H 2600 6800 50  0001 C CNN
	1    2600 6800
	0    1    1    0   
$EndComp
Wire Wire Line
	2600 6650 2800 6650
Wire Wire Line
	3000 6650 3000 7100
Wire Wire Line
	3000 7100 3150 7100
$Comp
L power:GND #PWR?
U 1 1 5D08E932
P 4300 8000
F 0 "#PWR?" H 4300 7750 50  0001 C CNN
F 1 "GND" H 4305 7827 50  0000 C CNN
F 2 "" H 4300 8000 50  0001 C CNN
F 3 "" H 4300 8000 50  0001 C CNN
	1    4300 8000
	0    1    1    0   
$EndComp
Wire Wire Line
	4300 8000 4350 8000
$Comp
L power:GND #PWR?
U 1 1 5D08F9E6
P 2600 7050
F 0 "#PWR?" H 2600 6800 50  0001 C CNN
F 1 "GND" H 2605 6877 50  0000 C CNN
F 2 "" H 2600 7050 50  0001 C CNN
F 3 "" H 2600 7050 50  0001 C CNN
	1    2600 7050
	1    0    0    -1  
$EndComp
Wire Wire Line
	2600 7050 2600 7000
$Comp
L Device:C C?
U 1 1 5D090C32
P 2300 6800
F 0 "C?" H 2415 6846 50  0000 L CNN
F 1 "C" H 2415 6755 50  0000 L CNN
F 2 "" H 2338 6650 50  0001 C CNN
F 3 "~" H 2300 6800 50  0001 C CNN
	1    2300 6800
	1    0    0    -1  
$EndComp
$Comp
L Device:C C?
U 1 1 5D091691
P 2300 6400
F 0 "C?" H 2415 6446 50  0000 L CNN
F 1 "C" H 2415 6355 50  0000 L CNN
F 2 "" H 2338 6250 50  0001 C CNN
F 3 "~" H 2300 6400 50  0001 C CNN
	1    2300 6400
	1    0    0    -1  
$EndComp
Wire Wire Line
	2300 6550 2300 6600
Wire Wire Line
	2300 6600 2500 6600
Wire Wire Line
	2500 6600 2500 7000
Wire Wire Line
	2500 7000 2600 7000
Connection ~ 2300 6600
Wire Wire Line
	2300 6600 2300 6650
Connection ~ 2600 7000
Wire Wire Line
	2600 7000 2600 6950
Wire Wire Line
	2300 6950 2300 7800
Wire Wire Line
	2300 7800 3100 7800
Wire Wire Line
	3100 7800 3800 7800
Connection ~ 3100 7800
Connection ~ 3800 7800
Wire Wire Line
	3800 7800 3800 7850
Text GLabel 5300 8750 1    50   Input ~ 0
Reset
Wire Wire Line
	5300 8800 5300 8750
Text GLabel 3250 7200 3    50   Input ~ 0
Reset
Wire Wire Line
	3250 7200 3250 7100
$Comp
L Connector_Generic:Conn_02x17_Odd_Even J?
U 1 1 5D0986DB
P 6000 9000
F 0 "J?" V 6004 8113 50  0000 R CNN
F 1 "Conn_02x17_Odd_Even" V 6095 8113 50  0000 R CNN
F 2 "" H 6000 9000 50  0001 C CNN
F 3 "~" H 6000 9000 50  0001 C CNN
	1    6000 9000
	0    1    1    0   
$EndComp
Text GLabel 5200 9400 3    50   Input ~ 0
Clock
Wire Wire Line
	5200 9400 5200 9300
Text GLabel 5550 7700 1    50   Input ~ 0
Clock
Wire Wire Line
	5750 8050 5550 8050
Wire Wire Line
	5550 8050 5550 7700
Text GLabel 3200 6100 3    50   Input ~ 0
Clock
Wire Wire Line
	3200 6100 3200 5850
$Comp
L reverse:MC14015BCP U?
U 1 1 5D0A2F1D
P 3550 4100
F 0 "U?" V 3479 4538 50  0000 L CNN
F 1 "MC14015BCP" V 3570 4538 50  0000 L CNN
F 2 "" H 3550 4100 50  0001 C CNN
F 3 "" H 3550 4100 50  0001 C CNN
	1    3550 4100
	0    1    1    0   
$EndComp
Text GLabel 3200 4550 3    50   Input ~ 0
Clock
Wire Wire Line
	3200 4550 3200 4450
Wire Wire Line
	4650 7650 4650 7700
Wire Wire Line
	3450 6400 3450 6300
Wire Wire Line
	3450 6300 3700 6300
NoConn ~ 3350 6400
Wire Wire Line
	3350 7100 3350 7150
Wire Wire Line
	3350 7150 3550 7150
Wire Wire Line
	4050 7150 4050 6600
$Comp
L power:GND #PWR?
U 1 1 5D0B2C66
P 4250 6900
F 0 "#PWR?" H 4250 6650 50  0001 C CNN
F 1 "GND" H 4255 6727 50  0000 C CNN
F 2 "" H 4250 6900 50  0001 C CNN
F 3 "" H 4250 6900 50  0001 C CNN
	1    4250 6900
	1    0    0    -1  
$EndComp
Wire Wire Line
	4050 6600 4250 6600
Wire Wire Line
	4250 6600 4250 6900
Connection ~ 4050 6600
Wire Wire Line
	3550 7100 3550 7150
Connection ~ 3550 7150
Wire Wire Line
	3550 7150 3650 7150
Wire Wire Line
	3650 7100 3650 7150
Connection ~ 3650 7150
Wire Wire Line
	3650 7150 3750 7150
Wire Wire Line
	3750 7100 3750 7150
Connection ~ 3750 7150
Wire Wire Line
	3750 7150 3850 7150
Wire Wire Line
	3850 7100 3850 7150
Connection ~ 3850 7150
Wire Wire Line
	3850 7150 4050 7150
$Comp
L power:+12V #PWR?
U 1 1 5D0BA2CA
P 4650 8350
F 0 "#PWR?" H 4650 8200 50  0001 C CNN
F 1 "+12V" H 4665 8523 50  0000 C CNN
F 2 "" H 4650 8350 50  0001 C CNN
F 3 "" H 4650 8350 50  0001 C CNN
	1    4650 8350
	-1   0    0    1   
$EndComp
Wire Wire Line
	4650 8350 4650 8300
$Comp
L power:+12V #PWR?
U 1 1 5D0BC225
P 2850 6550
F 0 "#PWR?" H 2850 6400 50  0001 C CNN
F 1 "+12V" H 2865 6723 50  0000 C CNN
F 2 "" H 2850 6550 50  0001 C CNN
F 3 "" H 2850 6550 50  0001 C CNN
	1    2850 6550
	1    0    0    -1  
$EndComp
Wire Wire Line
	2850 6550 2850 6600
Wire Wire Line
	2850 6600 2950 6600
NoConn ~ 3550 6400
NoConn ~ 3650 6400
Text GLabel 3900 5000 1    50   Input ~ 0
Clock
Wire Wire Line
	3700 5450 3400 5450
Wire Wire Line
	3400 5450 3400 5100
Wire Wire Line
	6250 4700 3700 4700
Wire Wire Line
	3700 4700 3700 5100
Wire Wire Line
	3500 5100 3500 4650
Wire Wire Line
	3500 4650 6450 4650
$Comp
L reverse:MC14015BCP U?
U 1 1 5D0D53EF
P 3550 3150
F 0 "U?" V 3479 3588 50  0000 L CNN
F 1 "MC14015BCP" V 3570 3588 50  0000 L CNN
F 2 "" H 3550 3150 50  0001 C CNN
F 3 "" H 3550 3150 50  0001 C CNN
	1    3550 3150
	0    1    1    0   
$EndComp
$Comp
L reverse:MC14015BCP U?
U 1 1 5D0D683F
P 3550 2250
F 0 "U?" V 3479 2688 50  0000 L CNN
F 1 "MC14015BCP" V 3570 2688 50  0000 L CNN
F 2 "" H 3550 2250 50  0001 C CNN
F 3 "" H 3550 2250 50  0001 C CNN
	1    3550 2250
	0    1    1    0   
$EndComp
$Comp
L Transistor_Array:ULN2004A U?
U 1 1 5D0E2248
P 8100 5400
F 0 "U?" V 8054 5930 50  0000 L CNN
F 1 "ULN2004A" V 8145 5930 50  0000 L CNN
F 2 "" H 8150 4850 50  0001 L CNN
F 3 "http://www.ti.com/lit/ds/symlink/uln2003a.pdf" H 8200 5200 50  0001 C CNN
	1    8100 5400
	0    1    1    0   
$EndComp
Wire Wire Line
	3600 5100 3600 4550
Wire Wire Line
	3600 4550 5850 4550
Wire Wire Line
	5850 4550 5850 3800
Wire Wire Line
	5850 3800 6750 3800
Wire Wire Line
	6750 3800 6750 3950
$Comp
L reverse:MC14049UBCP U?
U 1 1 5D0E8D5C
P 6950 4350
F 0 "U?" V 6904 4878 50  0000 L CNN
F 1 "MC14049UBCP" V 6995 4878 50  0000 L CNN
F 2 "" H 6850 4400 50  0001 C CNN
F 3 "" H 6850 4400 50  0001 C CNN
	1    6950 4350
	0    1    1    0   
$EndComp
Wire Wire Line
	6350 3900 6850 3900
Wire Wire Line
	6850 3900 6850 3950
Wire Wire Line
	6950 4750 6950 5000
Wire Wire Line
	6950 5000 6550 5000
Wire Wire Line
	3300 5100 3300 4900
Wire Wire Line
	3300 4900 3800 4900
Wire Wire Line
	6850 4900 6850 4750
Wire Wire Line
	3400 5850 3400 6150
Wire Wire Line
	3400 6150 6650 6150
Wire Wire Line
	3800 5100 3800 4900
Connection ~ 3800 4900
Wire Wire Line
	3800 4900 6850 4900
Wire Wire Line
	3500 5850 3500 5950
Wire Wire Line
	6650 5950 6650 4750
$Comp
L Device:Q_NPN_ECB Q?
U 1 1 5D1181D7
P 11150 6900
F 0 "Q?" V 11385 6900 50  0000 C CNN
F 1 "MJE802" V 11476 6900 50  0000 C CNN
F 2 "" H 11350 7000 50  0001 C CNN
F 3 "~" H 11150 6900 50  0001 C CNN
	1    11150 6900
	0    1    1    0   
$EndComp
$Comp
L Device:Q_NPN_ECB Q?
U 1 1 5D11D56F
P 11150 6000
F 0 "Q?" V 11385 6000 50  0000 C CNN
F 1 "MJE802" V 11476 6000 50  0000 C CNN
F 2 "" H 11350 6100 50  0001 C CNN
F 3 "~" H 11150 6000 50  0001 C CNN
	1    11150 6000
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5D11DFF3
P 11150 5750
F 0 "#PWR?" H 11150 5500 50  0001 C CNN
F 1 "GND" H 11155 5577 50  0000 C CNN
F 2 "" H 11150 5750 50  0001 C CNN
F 3 "" H 11150 5750 50  0001 C CNN
	1    11150 5750
	-1   0    0    1   
$EndComp
Wire Wire Line
	11150 5800 11150 5750
$Comp
L Device:Q_PNP_ECB Q?
U 1 1 5D1223CF
P 11950 6900
F 0 "Q?" V 12185 6900 50  0000 C CNN
F 1 "BD678" V 12276 6900 50  0000 C CNN
F 2 "" H 12150 7000 50  0001 C CNN
F 3 "~" H 11950 6900 50  0001 C CNN
	1    11950 6900
	0    1    1    0   
$EndComp
Wire Wire Line
	5700 9300 5800 9300
Wire Wire Line
	5700 8800 5800 8800
Wire Wire Line
	5800 9300 5800 8800
Connection ~ 5800 9300
Connection ~ 5800 8800
Wire Wire Line
	5500 8800 5600 8800
Wire Wire Line
	5600 8800 5600 9300
Connection ~ 5600 8800
Wire Wire Line
	5600 9300 5500 9300
Connection ~ 5600 9300
$Comp
L power:GND #PWR?
U 1 1 5D13A399
P 5600 9450
F 0 "#PWR?" H 5600 9200 50  0001 C CNN
F 1 "GND" H 5605 9277 50  0000 C CNN
F 2 "" H 5600 9450 50  0001 C CNN
F 3 "" H 5600 9450 50  0001 C CNN
	1    5600 9450
	1    0    0    -1  
$EndComp
Wire Wire Line
	5600 9450 5600 9300
$Comp
L power:+24V #PWR?
U 1 1 5D13F635
P 5800 8650
F 0 "#PWR?" H 5800 8500 50  0001 C CNN
F 1 "+24V" H 5815 8823 50  0000 C CNN
F 2 "" H 5800 8650 50  0001 C CNN
F 3 "" H 5800 8650 50  0001 C CNN
	1    5800 8650
	1    0    0    -1  
$EndComp
Wire Wire Line
	5800 8650 5800 8800
$Comp
L power:+24V #PWR?
U 1 1 5D143EE6
P 11650 7000
F 0 "#PWR?" H 11650 6850 50  0001 C CNN
F 1 "+24V" H 11665 7173 50  0000 C CNN
F 2 "" H 11650 7000 50  0001 C CNN
F 3 "" H 11650 7000 50  0001 C CNN
	1    11650 7000
	0    -1   -1   0   
$EndComp
$Comp
L Device:L L?
U 1 1 5D148EDB
P 13150 7000
F 0 "L?" V 12969 7000 50  0000 C CNN
F 1 "X0_Y0" V 13060 7000 50  0000 C CNN
F 2 "" H 13150 7000 50  0001 C CNN
F 3 "~" H 13150 7000 50  0001 C CNN
	1    13150 7000
	0    1    1    0   
$EndComp
Wire Wire Line
	11350 7000 11350 7400
Wire Wire Line
	11350 7400 12500 7400
Wire Wire Line
	12500 7400 12500 7000
Wire Wire Line
	12150 7000 12500 7000
Connection ~ 12500 7000
$Comp
L Device:L L?
U 1 1 5D152E7D
P 14800 7000
F 0 "L?" V 14619 7000 50  0000 C CNN
F 1 "X0_Y1" V 14710 7000 50  0000 C CNN
F 2 "" H 14800 7000 50  0001 C CNN
F 3 "~" H 14800 7000 50  0001 C CNN
	1    14800 7000
	0    1    1    0   
$EndComp
$Comp
L Device:Q_PNP_ECB BD678
U 1 1 5D1530EC
P 11950 6000
F 0 "BD678" V 12185 6000 50  0000 C CNN
F 1 "Q_PNP_ECB" V 12276 6000 50  0000 C CNN
F 2 "" H 12150 6100 50  0001 C CNN
F 3 "~" H 11950 6000 50  0001 C CNN
	1    11950 6000
	0    1    1    0   
$EndComp
$Comp
L power:+24V #PWR?
U 1 1 5D153C91
P 11950 5750
F 0 "#PWR?" H 11950 5600 50  0001 C CNN
F 1 "+24V" H 11965 5923 50  0000 C CNN
F 2 "" H 11950 5750 50  0001 C CNN
F 3 "" H 11950 5750 50  0001 C CNN
	1    11950 5750
	1    0    0    -1  
$EndComp
Wire Wire Line
	11950 5750 11950 5800
Wire Wire Line
	11350 6100 11350 6400
Wire Wire Line
	11350 6400 12450 6400
Wire Wire Line
	12450 6400 12450 6100
Wire Wire Line
	12450 6100 12150 6100
$Comp
L Device:L L?
U 1 1 5D165CC6
P 13100 6100
F 0 "L?" V 12919 6100 50  0000 C CNN
F 1 "X1_Y0" V 13010 6100 50  0000 C CNN
F 2 "" H 13100 6100 50  0001 C CNN
F 3 "~" H 13100 6100 50  0001 C CNN
	1    13100 6100
	0    1    1    0   
$EndComp
Connection ~ 12450 6100
$Comp
L Device:R_Network09 RN?
U 1 1 5D17A0D3
P 8450 7550
F 0 "RN?" V 7833 7550 50  0000 C CNN
F 1 "R_Network09" V 7924 7550 50  0000 C CNN
F 2 "Resistor_THT:R_Array_SIP10" V 9025 7550 50  0001 C CNN
F 3 "http://www.vishay.com/docs/31509/csc.pdf" H 8450 7550 50  0001 C CNN
	1    8450 7550
	0    1    -1   0   
$EndComp
$Comp
L power:+24V #PWR?
U 1 1 5D085865
P 8750 7950
F 0 "#PWR?" H 8750 7800 50  0001 C CNN
F 1 "+24V" H 8765 8123 50  0000 C CNN
F 2 "" H 8750 7950 50  0001 C CNN
F 3 "" H 8750 7950 50  0001 C CNN
	1    8750 7950
	0    1    1    0   
$EndComp
Wire Wire Line
	8650 7950 8750 7950
Wire Wire Line
	6850 7150 8250 7150
Wire Wire Line
	6750 7250 8250 7250
Wire Wire Line
	6650 7350 8250 7350
Wire Wire Line
	6550 7450 8250 7450
Wire Wire Line
	6450 7550 8250 7550
Wire Wire Line
	6350 7650 8250 7650
Wire Wire Line
	6250 7750 8250 7750
Wire Wire Line
	3900 5000 3900 5100
Wire Wire Line
	3600 5850 3600 6050
Wire Wire Line
	3600 6050 6850 6050
Wire Wire Line
	3500 5950 6650 5950
Wire Wire Line
	4100 5850 3900 5850
$Comp
L power:+12V #PWR?
U 1 1 5D0C5A51
P 4100 5850
F 0 "#PWR?" H 4100 5700 50  0001 C CNN
F 1 "+12V" H 4115 6023 50  0000 C CNN
F 2 "" H 4100 5850 50  0001 C CNN
F 3 "" H 4100 5850 50  0001 C CNN
	1    4100 5850
	0    1    1    0   
$EndComp
$Comp
L Device:R_Pack05_SIP RN?
U 1 1 5D1A15AC
P 9300 7550
F 0 "RN?" V 9308 7755 50  0000 L CNN
F 1 "R_Pack05_SIP" V 9399 7755 50  0000 L CNN
F 2 "Resistor_THT:R_Array_SIP10" V 10175 7550 50  0001 C CNN
F 3 "http://www.vishay.com/docs/31509/csc.pdf" H 9300 7550 50  0001 C CNN
	1    9300 7550
	0    1    1    0   
$EndComp
Wire Wire Line
	8250 7150 8900 7150
Wire Wire Line
	8900 7150 8900 6950
Wire Wire Line
	8900 6950 9100 6950
Connection ~ 8250 7150
$Comp
L Device:R_Pack05_SIP RN?
U 1 1 5D1C5B51
P 9950 7550
F 0 "RN?" V 9958 7755 50  0000 L CNN
F 1 "R_Pack05_SIP" V 10049 7755 50  0000 L CNN
F 2 "Resistor_THT:R_Array_SIP10" V 10825 7550 50  0001 C CNN
F 3 "http://www.vishay.com/docs/31509/csc.pdf" H 9950 7550 50  0001 C CNN
	1    9950 7550
	0    1    1    0   
$EndComp
Wire Wire Line
	8250 7250 9000 7250
Wire Wire Line
	9000 7250 9000 7150
Wire Wire Line
	9000 7150 9550 7150
Wire Wire Line
	9550 7150 9550 6950
Wire Wire Line
	9550 6950 9750 6950
Connection ~ 8250 7250
Wire Wire Line
	9100 7050 9000 7050
Wire Wire Line
	9000 7050 9000 6550
Wire Wire Line
	9000 6550 11950 6550
Wire Wire Line
	11950 6550 11950 6700
Wire Wire Line
	9750 7050 9650 7050
Wire Wire Line
	9650 7050 9650 6650
Wire Wire Line
	9650 6650 11150 6650
Wire Wire Line
	11150 6650 11150 6700
Wire Wire Line
	11650 7000 11750 7000
$Comp
L power:GND #PWR?
U 1 1 5D1F9599
P 10850 7000
F 0 "#PWR?" H 10850 6750 50  0001 C CNN
F 1 "GND" V 10855 6872 50  0000 R CNN
F 2 "" H 10850 7000 50  0001 C CNN
F 3 "" H 10850 7000 50  0001 C CNN
	1    10850 7000
	0    1    1    0   
$EndComp
Wire Wire Line
	10850 7000 10950 7000
$Comp
L Device:D D?
U 1 1 5D201409
P 13400 7200
F 0 "D?" V 13446 7121 50  0000 R CNN
F 1 "D" V 13355 7121 50  0000 R CNN
F 2 "" H 13400 7200 50  0001 C CNN
F 3 "~" H 13400 7200 50  0001 C CNN
	1    13400 7200
	0    -1   -1   0   
$EndComp
$Comp
L Device:D D?
U 1 1 5D220D39
P 13650 7000
F 0 "D?" H 13650 6784 50  0000 C CNN
F 1 "D" H 13650 6875 50  0000 C CNN
F 2 "" H 13650 7000 50  0001 C CNN
F 3 "~" H 13650 7000 50  0001 C CNN
	1    13650 7000
	-1   0    0    1   
$EndComp
Text GLabel 13900 7000 2    50   Input ~ 0
Clear
Wire Wire Line
	13800 7000 13900 7000
Wire Wire Line
	13300 7000 13400 7000
Wire Wire Line
	13400 7000 13400 7050
Connection ~ 13400 7000
Wire Wire Line
	13400 7000 13500 7000
Text GLabel 13400 7450 3    50   Input ~ 0
Y0
Wire Wire Line
	13400 7350 13400 7450
$Comp
L Device:C C?
U 1 1 5D23FA4C
P 3250 8950
F 0 "C?" H 3365 8996 50  0000 L CNN
F 1 "C" H 3365 8905 50  0000 L CNN
F 2 "" H 3288 8800 50  0001 C CNN
F 3 "~" H 3250 8950 50  0001 C CNN
	1    3250 8950
	1    0    0    -1  
$EndComp
Wire Wire Line
	3250 8800 2800 8800
Wire Wire Line
	2800 6650 2800 8450
Connection ~ 2800 6650
Wire Wire Line
	2800 6650 3000 6650
$Comp
L power:GND #PWR?
U 1 1 5D2500AE
P 3250 9200
F 0 "#PWR?" H 3250 8950 50  0001 C CNN
F 1 "GND" H 3255 9027 50  0000 C CNN
F 2 "" H 3250 9200 50  0001 C CNN
F 3 "" H 3250 9200 50  0001 C CNN
	1    3250 9200
	1    0    0    -1  
$EndComp
Wire Wire Line
	3250 9200 3250 9100
Wire Wire Line
	5750 8300 5650 8300
Wire Wire Line
	5650 8300 5650 8450
Wire Wire Line
	5650 8450 2800 8450
Connection ~ 2800 8450
Wire Wire Line
	2800 8450 2800 8800
Text GLabel 5750 7650 1    50   Input ~ 0
Reset
Wire Wire Line
	5750 7850 5750 7650
$Comp
L power:GND #PWR?
U 1 1 5D26930D
P 5100 8650
F 0 "#PWR?" H 5100 8400 50  0001 C CNN
F 1 "GND" H 5105 8477 50  0000 C CNN
F 2 "" H 5100 8650 50  0001 C CNN
F 3 "" H 5100 8650 50  0001 C CNN
	1    5100 8650
	0    1    1    0   
$EndComp
Wire Wire Line
	5200 8800 5200 8650
Wire Wire Line
	5200 8650 5100 8650
Wire Wire Line
	6250 6250 6250 4700
Wire Wire Line
	6350 6250 6350 3900
Wire Wire Line
	6450 4650 6450 6250
Wire Wire Line
	6550 5000 6550 6250
Wire Wire Line
	6650 6150 6650 6250
Wire Wire Line
	6750 4750 6750 6250
Wire Wire Line
	6850 7050 6850 7150
Wire Wire Line
	6750 7050 6750 7250
Wire Wire Line
	6650 7050 6650 7350
Wire Wire Line
	6550 7050 6550 7450
Wire Wire Line
	6450 7050 6450 7550
Wire Wire Line
	6350 7050 6350 7650
Wire Wire Line
	6250 7050 6250 7750
Wire Wire Line
	6850 6050 6850 6250
$Comp
L Transistor_Array:ULN2004A U?
U 1 1 5D078AF8
P 6650 6650
F 0 "U?" V 6604 7180 50  0000 L CNN
F 1 "ULN2004A" V 6695 7180 50  0000 L CNN
F 2 "" H 6700 6100 50  0001 L CNN
F 3 "http://www.ti.com/lit/ds/symlink/uln2003a.pdf" H 6750 6450 50  0001 C CNN
	1    6650 6650
	0    1    1    0   
$EndComp
Text GLabel 3900 7350 2    50   Input ~ 0
Board_select
Wire Wire Line
	3800 7350 3900 7350
Connection ~ 3800 7350
Text GLabel 5000 7700 1    50   Input ~ 0
Board_select
Wire Wire Line
	5000 7850 5050 7850
Wire Wire Line
	5000 7700 5000 7850
$Comp
L power:+12V #PWR?
U 1 1 5D36831A
P 6100 7800
F 0 "#PWR?" H 6100 7650 50  0001 C CNN
F 1 "+12V" H 6115 7973 50  0000 C CNN
F 2 "" H 6100 7800 50  0001 C CNN
F 3 "" H 6100 7800 50  0001 C CNN
	1    6100 7800
	1    0    0    -1  
$EndComp
Wire Wire Line
	6100 7800 6100 7850
Connection ~ 5400 8000
Wire Wire Line
	5400 8000 5400 7850
$Comp
L Device:CP C?
U 1 1 5D07FBE7
P 5400 8150
F 0 "C?" H 5400 8050 50  0000 R CNN
F 1 "1uF" H 5550 8250 50  0000 R CNN
F 2 "" H 5438 8000 50  0001 C CNN
F 3 "~" H 5400 8150 50  0001 C CNN
	1    5400 8150
	-1   0    0    1   
$EndComp
Wire Wire Line
	5050 8000 5400 8000
$Comp
L Device:CP C?
U 1 1 5D080F32
P 5050 8150
F 0 "C?" H 5050 8050 50  0000 R CNN
F 1 "10uF" H 5250 8250 50  0000 R CNN
F 2 "" H 5088 8000 50  0001 C CNN
F 3 "~" H 5050 8150 50  0001 C CNN
	1    5050 8150
	-1   0    0    1   
$EndComp
Connection ~ 4650 8300
Wire Wire Line
	4650 8300 5050 8300
Wire Wire Line
	5400 8300 5400 8350
Wire Wire Line
	5400 8350 4850 8350
Wire Wire Line
	4850 8350 4850 7650
Wire Wire Line
	4850 7650 4650 7650
Connection ~ 5400 8350
Wire Wire Line
	5400 8350 5400 8800
Text GLabel 4850 7550 1    50   Input ~ 0
V_In
Wire Wire Line
	4850 7550 4850 7650
Connection ~ 4850 7650
Wire Wire Line
	6050 8050 6050 8300
$Comp
L power:GND #PWR?
U 1 1 5D456C04
P 6050 8350
F 0 "#PWR?" H 6050 8100 50  0001 C CNN
F 1 "GND" H 6055 8177 50  0000 C CNN
F 2 "" H 6050 8350 50  0001 C CNN
F 3 "" H 6050 8350 50  0001 C CNN
	1    6050 8350
	1    0    0    -1  
$EndComp
Wire Wire Line
	6050 8350 6050 8300
Connection ~ 6050 8300
Text GLabel 5400 9400 3    50   Input ~ 0
DoNothing
Wire Wire Line
	5400 9400 5400 9300
Text GLabel 3050 8100 0    50   Input ~ 0
DoNothing
Wire Wire Line
	3050 8100 3100 8100
NoConn ~ 5300 9300
Text GLabel 5900 9400 3    50   Input ~ 0
Clear
Wire Wire Line
	5900 9400 5900 9300
$Comp
L Device:Q_NPN_ECB Q?
U 1 1 5D521903
P 11150 5000
F 0 "Q?" V 11385 5000 50  0000 C CNN
F 1 "MJE802" V 11476 5000 50  0000 C CNN
F 2 "" H 11350 5100 50  0001 C CNN
F 3 "~" H 11150 5000 50  0001 C CNN
	1    11150 5000
	0    1    1    0   
$EndComp
$Comp
L Device:Q_PNP_ECB BD678
U 1 1 5D522078
P 11950 5000
F 0 "BD678" V 12185 5000 50  0000 C CNN
F 1 "Q_PNP_ECB" V 12276 5000 50  0000 C CNN
F 2 "" H 12150 5100 50  0001 C CNN
F 3 "~" H 11950 5000 50  0001 C CNN
	1    11950 5000
	0    1    1    0   
$EndComp
Wire Wire Line
	12500 7000 12500 6550
Wire Wire Line
	14550 6550 14550 7000
Wire Wire Line
	14550 7000 14650 7000
$Comp
L Device:D D?
U 1 1 5D52B6D5
P 15050 7200
F 0 "D?" V 15096 7121 50  0000 R CNN
F 1 "D" V 15005 7121 50  0000 R CNN
F 2 "" H 15050 7200 50  0001 C CNN
F 3 "~" H 15050 7200 50  0001 C CNN
	1    15050 7200
	0    -1   -1   0   
$EndComp
$Comp
L Device:D D?
U 1 1 5D52C0BD
P 15300 7000
F 0 "D?" H 15300 6784 50  0000 C CNN
F 1 "D" H 15300 6875 50  0000 C CNN
F 2 "" H 15300 7000 50  0001 C CNN
F 3 "~" H 15300 7000 50  0001 C CNN
	1    15300 7000
	-1   0    0    1   
$EndComp
Text GLabel 15550 7000 2    50   Input ~ 0
Clear
Text GLabel 15050 7450 3    50   Input ~ 0
Y1
Wire Wire Line
	14950 7000 15050 7000
Wire Wire Line
	15050 7000 15050 7050
Connection ~ 15050 7000
Wire Wire Line
	15050 7000 15150 7000
Wire Wire Line
	15050 7350 15050 7450
Wire Wire Line
	15450 7000 15550 7000
Text GLabel 6800 8750 1    50   Input ~ 0
Y0
Wire Wire Line
	6800 8750 6800 8800
Text GLabel 6800 9350 3    50   Input ~ 0
Y1
Wire Wire Line
	6800 9350 6800 9300
Wire Wire Line
	3700 5450 3700 6300
Text Notes 12800 4850 0    118  ~ 0
Actual Flipdot Panel
Wire Wire Line
	12450 6100 12950 6100
Wire Wire Line
	12500 6550 14550 6550
Wire Wire Line
	12500 7000 13000 7000
Wire Notes Line
	12800 4950 15900 4950
Wire Notes Line
	15900 4950 15900 7750
Wire Notes Line
	15900 7750 12800 7750
Wire Notes Line
	12800 7750 12800 4950
$EndSCHEMATC
