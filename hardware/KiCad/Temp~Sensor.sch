EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
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
L Sensor_Temperature:DS18B20 U1
U 1 1 6179A3FC
P 4100 4450
F 0 "U1" H 3870 4404 50  0000 R CNN
F 1 "DS18B20" H 3870 4495 50  0000 R CNN
F 2 "TO_SOT_Packages_THT:TO-92_Molded_Wide_Reverse" H 3100 4200 50  0001 C CNN
F 3 "http://datasheets.maximintegrated.com/en/ds/DS18B20.pdf" H 3950 4700 50  0001 C CNN
	1    4100 4450
	-1   0    0    1   
$EndComp
$Comp
L conn:Screw_Terminal_01x03 J1
U 1 1 6179AF65
P 5350 3700
F 0 "J1" H 5430 3742 50  0000 L CNN
F 1 "Screw_Terminal_01x03" H 5430 3651 50  0000 L CNN
F 2 "Connectors_Phoenix:PhoenixContact_MCV-G_03x3.50mm_Vertical" H 5350 3700 50  0001 C CNN
F 3 "~" H 5350 3700 50  0001 C CNN
	1    5350 3700
	1    0    0    -1  
$EndComp
$Comp
L conn:Conn_01x03_Female J3
U 1 1 6179D6C6
P 5400 5150
F 0 "J3" H 5428 5176 50  0000 L CNN
F 1 "Conn_01x03_Female" H 5428 5085 50  0000 L CNN
F 2 "MyParts:RSProClamp" H 5400 5150 50  0001 C CNN
F 3 "~" H 5400 5150 50  0001 C CNN
	1    5400 5150
	1    0    0    -1  
$EndComp
$Comp
L conn:Conn_01x03_Female J2
U 1 1 6179E016
P 5400 4450
F 0 "J2" H 5428 4476 50  0000 L CNN
F 1 "Conn_01x03_Female" H 5428 4385 50  0000 L CNN
F 2 "MyParts:Amphenol_FCI_3" H 5400 4450 50  0001 C CNN
F 3 "~" H 5400 4450 50  0001 C CNN
	1    5400 4450
	1    0    0    -1  
$EndComp
Wire Wire Line
	4100 4150 4100 3600
Wire Wire Line
	4100 3600 4850 3600
Wire Wire Line
	4850 3600 4850 4350
Wire Wire Line
	4850 4350 5200 4350
Connection ~ 4850 3600
Wire Wire Line
	4850 3600 5150 3600
Wire Wire Line
	4850 4350 4850 5050
Wire Wire Line
	4850 5050 5200 5050
Connection ~ 4850 4350
Wire Wire Line
	4100 4750 4100 5250
Wire Wire Line
	4100 5250 4700 5250
Wire Wire Line
	4700 5250 4700 4550
Wire Wire Line
	4700 4550 5200 4550
Connection ~ 4700 5250
Wire Wire Line
	4700 5250 5200 5250
Wire Wire Line
	4700 4550 4700 3800
Wire Wire Line
	4700 3800 5150 3800
Connection ~ 4700 4550
Wire Wire Line
	3800 3700 4550 3700
Wire Wire Line
	4550 3700 4550 4450
Wire Wire Line
	4550 4450 5200 4450
Connection ~ 4550 3700
Wire Wire Line
	4550 3700 5150 3700
Wire Wire Line
	4550 4450 4550 5150
Wire Wire Line
	4550 5150 5200 5150
Connection ~ 4550 4450
$Comp
L Sensor_Temperature:DS18B20 U2
U 1 1 617A717B
P 3350 4450
F 0 "U2" H 3120 4404 50  0000 R CNN
F 1 "DS18B20" H 3120 4495 50  0000 R CNN
F 2 "TO_SOT_Packages_THT:TO-92_Molded_Wide_Reverse" H 2350 4200 50  0001 C CNN
F 3 "http://datasheets.maximintegrated.com/en/ds/DS18B20.pdf" H 3200 4700 50  0001 C CNN
	1    3350 4450
	-1   0    0    1   
$EndComp
Wire Wire Line
	3800 4450 3800 3900
Wire Wire Line
	3350 4150 4100 4150
Connection ~ 4100 4150
Wire Wire Line
	3350 4750 4100 4750
Connection ~ 4100 4750
Wire Wire Line
	3050 3900 3050 4450
Wire Wire Line
	3050 3900 3800 3900
Connection ~ 3800 3900
Wire Wire Line
	3800 3900 3800 3700
$Comp
L device:R_Small R1
U 1 1 617B1F7B
P 2650 4600
F 0 "R1" H 2709 4646 50  0000 L CNN
F 1 "R_Small" H 2709 4555 50  0000 L CNN
F 2 "Resistors_SMD:R_0603_HandSoldering" H 2650 4600 50  0001 C CNN
F 3 "~" H 2650 4600 50  0001 C CNN
	1    2650 4600
	1    0    0    -1  
$EndComp
$Comp
L device:LED D1
U 1 1 617B31FB
P 2650 4200
F 0 "D1" V 2597 4280 50  0000 L CNN
F 1 "LED" V 2688 4280 50  0000 L CNN
F 2 "LEDs:LED_0603_HandSoldering" H 2650 4200 50  0001 C CNN
F 3 "~" H 2650 4200 50  0001 C CNN
	1    2650 4200
	0    1    1    0   
$EndComp
Wire Wire Line
	3350 4750 2650 4750
Wire Wire Line
	2650 4750 2650 4700
Connection ~ 3350 4750
Wire Wire Line
	2650 4500 2650 4350
Wire Wire Line
	2650 4050 2650 3900
Wire Wire Line
	2650 3900 3050 3900
Connection ~ 3050 3900
$EndSCHEMATC
