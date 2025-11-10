# orbitNP.py
#  
# Author: Matthew Wilkinson.
# Institute: Space Geodesy Facility, Herstmonceux UK.
# Research Council: British Geological Survey, Natural Environment Research Council.
# 
# Version: 1.0
# Last Modified: 8th December 2016
# 
# Please visit here to read our disclaimer: http://www.bgs.ac.uk/downloads/softdisc.html and please refer to the LICENSE.txt document included in this distribution.

# Please refer to the README file included in this distribution.


import matplotlib

matplotlib.use('TkAgg')

# import PyQt5
# matplotlib.use('qt5agg')

# import PyQt4
# matplotlib.use('qt4agg')


import datetime as dt
import time
import os
import subprocess
import sys, getopt

from matplotlib.pyplot import *
import matplotlib.dates as md
import matplotlib.gridspec as gridspec
from matplotlib.dates import date2num, DateFormatter, AutoDateFormatter, AutoDateLocator

import numpy
import scipy
from scipy.stats import skew, kurtosis
from scipy import interpolate
from scipy.optimize import curve_fit
import math
from decimal import *

#warnings.simplefilter(action='ignore', category=FutureWarning)


###
# Program Functions
###

def refmm(pres, temp, hum, alt, rlam, phi, hm):
    # Calculate and return refraction delay range corrections from Marini-Murray model using the pressure,
    # temperature, humidity, satellite elevation, station latitude, station height and laser wavelength

    flam = 0.9650 + 0.0164 / (rlam * rlam) + 0.228e-3 / (rlam ** 4)
    fphih = 1.0 - 0.26e-2 * np.cos(2.0 * phi) - 0.3 * hm
    tzc = temp - 273.15
    ez = hum * 6.11e-2 * (10.0 ** ((7.5 * tzc) / (237.3 + tzc)))
    rk = 1.163 - 0.968e-2 * np.cos(2.0 * phi) - 0.104e-2 * temp + 0.1435e-4 * pres
    a = 0.2357e-2 * pres + 0.141e-3 * ez
    b = 1.084e-8 * pres * temp * rk + (4.734e-8 * 2.0 * pres * pres) / (temp * (3.0 - 1.0 / rk))
    sine = np.sin(alt * 2.0 * np.pi / 360.0)
    ab = a + b
    delr = (flam / fphih) * (ab / (sine + (b / ab) / (sine + 0.01)))

    tref = delr * 2.0  # 2-way delay in meters

    return tref


def dchols(a, m):
    # Perform Choleski matrix inversion

    # JRM mod in all file, float128 replaced by np.longdouble
    s = np.zeros([m, m], dtype=np.longdouble)
    b = np.zeros([m, m], dtype=np.longdouble)
    x = np.zeros(m, dtype=np.longdouble)

    ierr = 0
    arng = np.arange(m)

    sel = np.where(a[arng, arng] <= 0.0)
    if (np.size(sel) > 0):
        ierr = 2
        return ierr, a

    s[:, arng] = 1.0 / np.sqrt(a[arng, arng])
    a = a * s * s.transpose()

    for i in range(m):
        sum = a[i, i]
        if (i > 0):
            for k in range(i):
                sum = sum - b[k, i] ** 2

        if (sum <= 0.0):
            ierr = 3
            return ierr, a

        b[i, i] = np.sqrt(sum)

        if (i != m - 1):
            for j in range(i + 1, m):
                sum = a[j, i]
                if (i > 0):
                    for k in range(i):
                        sum = sum - b[k, i] * b[k, j]
                b[i, j] = sum / b[i, i]

    for i in range(m):
        for j in range(m):
            x[j] = 0.0
        x[i] = 1.0
        for j in range(m):
            sum = x[j]
            if (j > 0):
                for k in range(j):
                    sum = sum - b[k, j] * x[k]
            x[j] = sum / b[j, j]

        for j in range(m):
            m1 = m - 1 - j
            sum = x[m1]
            if (j > 0):
                for k in range(j):
                    m2 = m - 1 - k
                    sum = sum - b[m1, m2] * x[m2]
            x[m1] = sum / b[m1, m1]

        for j in range(m):
            a[j, i] = x[j]

    reta = a * s * s.transpose()

    return ierr, reta


def mjd2cal(mjd):
    # Return the Modified Julian Day from the calendar year, month and day of month
    dym = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
    mil = 51543.0
    remd = mjd - mil

    if remd > 0:
        i = 0
        while remd > 366.0:
            if np.mod(i, 4) == 0:
                remd = remd - 366.0
            else:
                remd = remd - 365.0
            i = i + 1
    else:
        i = 0
        while remd < 0.0:
            if np.mod(i - 1, 4) == 0:
                remd = remd + 366.0
            else:
                remd = remd + 365.0
            i = i - 1

    yy = 2000 + i

    if np.mod(yy, 4) == 0:
        dym[1] = 29

    i = 0
    while (remd > dym[i]):
        remd = remd - dym[i]
        i = i + 1

    mm = i + 1
    dd = int(remd)
    return yy, mm, dd


def cal2mjd(yr, mm, dd):
    # Return the Modified Julian Day from the calendar year, month and day of month
    dym = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
    mil = 51543.0
    sumd = mil

    yy = yr - 2000
    if yy > 0:
        i = 0
        while i < yy:
            if np.mod(i, 4) == 0:
                sumd = sumd + 366.0
            else:
                sumd = sumd + 365.0
            i = i + 1
    else:
        i = -1
        while i >= yy:
            if np.mod(i, 4) == 0:
                sumd = sumd - 366.0
            else:
                sumd = sumd - 365.0
            i = i - 1

    if np.mod(yy, 4) == 0:
        dym[1] = 29

    i = 0
    while (i + 1 < mm):
        sumd = sumd + dym[i]
        i = i + 1

    sumd = sumd + dd
    return sumd


###
###


###
# Main Program
###

# OrbitNP accepts inputs on the python command line.  The following code reads the submitted parameters and
# assigns them to the appropriate variables.

ERdata = ''  # Epoch-Range datafile
frdata = ''  # ILRS full rate date file in CRD format
CPFin = ''  # ILRS XYZ orbit prediction in CPF format
pstr = ''  # 3-character string to select the prediction provider
METin = ''  # File containing the local meteorological data
CALin = ''  # File containing the system delay values
INmjd = 0.0  # Input the MJD for raw data sets
STAT_id = ''  # File containing the local meteorological data
STAT_name = ''  # Station name
SATtarget_name = ''  # Satellite name
savepassname = ''  # Satellite name
frate = 0.0  # Laser fire rate
autoCPF = False  # Flag to attempt to automatically fetch the ILRS CPF file according to the full rate data filename
METap = False  # Flag to show met data applied
PWidth = 0.0
PWadjust = False  # Adjust Gaussian fit by considering laser pulse width
plotRES = False  # Flag to plot and save the results, no display
displayRES = False  # Flag to display and save the results
clipsigma = False  # Set residual clipping  wrt to calculated RMS
clipLEHM = False  # Set residual clipping infront and behind the LEHM
NPout = False  # Output range residuals to file
RRout = False  # Output range residuals to file
SLVout = False  # Output orbit parameters to file
Unfilter = False  # Flag to include the data in the CRD full-rate data that is flagged as 'noise'
ipass = -1  # select pass in FRD file
setupWarningList = []  # setup warnings output in summary
runWarningList = []  # run warnings output in summary

NPbin_length = -1.0  # Set length of normal point in seconds
minNPn = -1  # Set the minimum number of range observations to form a Normal point

loop = True  # loop in a FRD file
FRDloop = False  # set FRD file loop

a = scipy.__version__.split('.')
if ((int(a[0]) == 0) & (int(a[1]) < 17)):
    print("Error: Scipy library version is too old. Version 0.17.0 or later is required")
    sys.exit()

refSTid = np.array(['7840',  # Herstmonceux
                    '7821',  # Shanghai
                    '7841',  # Potsdam
                    '7237',  # Changchun
                    '7249',  # Beijing
                    '7825',  # Mt Stromlo
                    '7941',  # Matera
                    '7839',  # Graz
                    '7811',  # Borowiec
                    '1884',  # Riga
                    '7838',  # Simosato
                    '1891',  # Irkutsk
                    '1824',  # Golosiiv
                    '1873',  # Simeiz
                    '1888',  # Svetloe
                    '1889',  # Zelenchukskya
                    '8834',  # Wettzell (WLRS)
                    '7827',  # Wettzell (SOS-W)
                    '7090',  # Yarragadee
                    '7501',  # Hartebeesthoek
                    '7403',  # Arequipa
                    '7119',  # Haleakala
                    '7105',  # Greenbelt
                    '7110',  # Monument Peak
                    '7810'])  # Zimmerwald

refSTname = np.array(['Herstmonceux',  # Herstmonceux
                      'Shanghai',  # Shanghai
                      'Potsdam',  # Potsdam
                      'Changchun',  # Changchun
                      'Beijing',  # Beijing
                      'Mt Stromlo',  # Mt Stromlo
                      'Matera',  # Matera
                      'Graz',  # Graz
                      'Borowiec',  # Borowiec
                      'Riga',  # Riga
                      'Simosato',  # Simosato
                      'Irkutsk',  # Irkutsk
                      'Golosiiv',  # Golosiiv
                      'Simeiz',  # Simeiz
                      'Svetloe',  # Svetloe
                      'Zelenchukskya',  # Zelenchukskya
                      'Wettzell (WLRS)',  # Wettzell (WLRS)
                      'Wettzell (SOS-W)',  # Wettzell (SOS-W)
                      'Yarragadee',  # Yarragadee
                      'Hartebeesthoek',  # Hartebeesthoek
                      'Arequipa',  # Arequipa
                      'Haleakala',  # Haleakala
                      'Greenbelt',  # Greenbelt
                      'Monument Peak',  # Monument Peak
                      'Zimmerwald'])  # Zimmerwald

refSTcoords = np.array(['50.867387 0.336129 75.357',  # Herstmonceux
                        '31.09617 121.1866 99.961',  # Shanghai
                        '52.3830 13.0614 123.5',  # Potsdam
                        '43.7905 125.4435 274.2',  # Changchun
                        '39.6069 115.8920 81.666',  # Beijing
                        '-35.3161 149.0099 805.0',  # Mt Stromlo
                        '40.6486 16.7046 536.9',  # Matera
                        '47.06711111 15.49333333 541.1',  # Graz
                        '52.2770 17.0746 123.4',  # Borowiec
                        '56.948551 24.059075 31.3367',  # Riga
                        '33.5777 135.9370 62.44',  # Simosato
                        '52.2191 104.3164 505.62',  # Irkutsk
                        '50.3633 30.4961 212.9',  # Golosiiv
                        '44.4128 33.9931 361.20',  # Simeiz
                        '60.5332 29.7805 69.0',  # Svetloe
                        '43.7887 41.5654 1155.4',  # Zelenchukskya
                        '49.1444 12.8780 665',  # Wettzell (WLRS)
                        '49.1449402 12.8781000  663.174',  # Wettzell (SOS-W)
                        '-29.0464 115.3467 244',  # Yarragadee
                        '-25.8897 27.6861 1406.822',  # Hartebeesthoek
                        '-16.4657 -71.4930 2489.05',  # Arequipa
                        '20.706489 203.743079 3056.272',  # Haleakala
                        '39.0206 -76.82770 19.184',  # Greenbelt
                        '32.8917 -116.4227 1842.177',  # Monument Peak
                        '46.8772 7.4652 951.2'])  # Zimmerwald

print('\n -- Check input parameters')

try:
    opts, args = getopt.getopt(sys.argv[1:],
                               "hf:d:c:Am:p:j:H:w:N:M:b:xXz:l:k:u::s:t:onrve")  # ,["ffile","dfile","cfile","mfile","pstr","INmjd","Hz","NPsec","LatlongAlt","clip","STid"])
except getopt.GetoptError:
    print('Error: Check all input options requirements')
    print('orbitNP.py -h for help option')
    sys.exit()

if np.size(opts) == 0:
    print('Error: No arguments given. Printing help options...')
    opts = [('-h', '')]

for opt, arg in opts:
    if opt == '-h':
        print(
            '\n\torbitNP.py - An example program to adjust CPF orbits to flatten SLR data and output normal points.\n\n'
            '\tInput epoch-range data file, either in ILRS FRD format or raw epoch-range data.\n'
            '\tInput the corresponding CPF prediction file or let the program search for the corresponding CPF.\n'
            '\tInput meteorological data as an epoch, pressure, temperature, humidity file if using raw data.\n\n'

            '\tOptions:\n'
            '\t  -f <FRD file>      \tFull rate FRD file format file\n'
            '\t  -c <CPF file>      \tCPF prediction file\n'
            '\t  -p <provider>      \tCPF prediction provider (3 character string)\n'
            '\t  -d <datafile>      \tRaw Epoch-Range data file\n'
            '\t  -m <Met File>      \tLocal Meteorological datafile\n'
            '\t                     \t   FORMAT: [Mjd] [Seconds of Day] [Pressure] [Temperature(K)] [Humidity] e.g.\n '
            '\t  -b <Cal File>      \tSystem Delay Calibration to be applied\n'
            '\t                     \t   FORMAT: [Mjd] [Seconds of Day] [Two-Way System Calibration in picoseconds]\n '
            '\t                     \t   57395 11638.080 998.32 276.20  87.6\n'
            '\t  -j <mjd>           \tModified Julian Day of raw data at start of observations\n'
            '\t  -s <Station code>  \tstation abbreviation or 4-digit code\n'
            '\t  -t <target name>   \tsatellite target name used to fetch CPF\n'
            '\t  -l <"Lat Lon Alt"> \tString containing station latitude(deg), longitude(deg) and altitude(m)\n'
            '\t  -A                 \tauto find CPF Prediction\n'
            '\t  -H <Hz>            \tlaser repetition rate\n'
            '\t  -w <ps>            \tlaser pulse width\n'
            '\t  -N <seconds>       \tNormal point length in seconds, Default 30 seconds\n'
            '\t  -M <number>        \tminimum number of range observations to form a Normal point. Default 30\n'
            '\t  -e                 \tInclude unfiltered range measurements\n'
            '\t  -k <factor>        \tApply factor*sigma clipping to flattened residuals before forming normal points [2.0 - 5.0 permitted]\n'
            '\t  -u <lower:upper>   \tApply fixed clipping from the LEHM of the distribution at <lower> and <upper> limits in ps\n'
            '\t  -o                 \tReturn to pass selection in FRD file on analysis completion\n'
            '\t  -n                 \tOutput normal points to file normalp.dat in CRD format\n'
            '\t  -r                 \tOutput range residuals to file resids.dat\n'
            '\t  -v                 \tOutput obit solve parameters to file solvep.out\n'
            '\t  -x                 \tPlot final results and save as .png, no display\n'
            '\t  -X                 \tPlot final results, display and save as .png in \'pics\' folder\n'
            '\t  -z                 \tSpecify filename, without .png extension\n\n'
            '  Examples:\n'
            '\t run orbitNP.py -f lageos1_201606.frd -A -x -s 7237 -N 120 -o -r\n'
            '\t run orbitNP.py -d epochrange.dat -c ajisai_cpf_181010_7831.jax -m met.dat -u -100:500 -x -s 7840 -N 30 -n -j 58401 -t Ajisai\n\n')
        sys.exit()
    elif opt in ("-f"):
        frdata = arg
    elif opt in ("-c"):
        CPFin = arg
    elif opt in ("-p"):
        pstr = arg
    elif opt in ("-d"):
        ERdata = arg
    elif opt in ("-m"):
        METin = arg
    elif opt in ("-b"):
        CALin = arg
    elif opt in ("-j"):
        INmjd = float(arg)
    elif opt in ("-l"):
        STAT_coords = arg
    elif opt in ("-H"):
        frate = float(arg)
    elif opt in ("-w"):
        PWidth = float(arg)
        PWadjust = True
    elif opt in ("-N"):
        NPbin_length = float(arg)
    elif opt in ("-M"):
        minNPn = float(arg)
    elif opt in ("-e"):
        Unfilter = True
    elif opt in ("-s"):
        flag = False
        STAT_id = arg

        for opt, arg in opts:
            if(opt in ("-l")):
                flag = True
        if not(flag):
            sel = np.where(STAT_id == refSTid)
            if (np.size(sel)):  # Angel Vera mod
                print("-s: No Station co-ordinates for ID number:", STAT_id)
                sys.exit()
            STAT_coords = refSTcoords[sel][0]
            STAT_name = refSTname[sel][0]

    elif opt in ("-t"):
        SATtarget_name = arg
    elif opt in ("-A"):
        autoCPF = True
    elif opt in ("-k"):
        clipsigma = True
        try:
            cfactor = float(arg)
        except:
            print("-n: Failed to read sigma factor float")
            sys.exit()

        if (cfactor < 2.0) | (cfactor > 5.0):
            print("-r: Sigma rejection factor outside of permitted limits [2.0 - 5.0]")
            sys.exit()
    elif opt in ("-u"):
        clipLEHM = True
        a = arg.split(':')
        if (np.size(a) != 2):
            print("-u: Incorrect LEHM lower and upper input values")
            sys.exit()
        LEHMlow = np.min([float(a[0]), float(a[1])])
        LEHMupp = np.max([float(a[0]), float(a[1])])
        if (LEHMlow > 0.0):
            print('\t ** Clipping not set below the LEHM')
            setupWarningList.append(
                "Clipping not set below the LEHM, suggest '-u -" + str(int(LEHMlow)) + ':' + str(int(LEHMupp)) + "'")
    elif opt in ("-o"):
        FRDloop = True
    elif opt in ("-n"):
        NPout = True
    elif opt in ("-r"):
        RRout = True
    elif opt in ("-v"):
        SLVout = True
    elif opt in ("-x"):
        plotRES = True
    elif opt in ("-X"):
        plotRES = True
        displayRES = True
    elif opt in ("-z"):
        savepassname = arg

if (STAT_coords == ''):
    print("-s: No Station co-ordinates or matching Station ID provided")
    sys.exit()
else:
    a = STAT_coords.split()
    STAT_LAT = float(a[0])  # Station Latitude
    STAT_LONG = float(a[1])  # Station Longitude
    STAT_HEI = float(a[2])  # Station HEIGHT ABOVE SPHEROID IN METRES

if (frdata != '') & (ERdata != ''):
    print("\t- Two laser range data files provided")
    sys.exit()

if (STAT_id == '') & (STAT_coords == ''):
    print("\t- No Station ID or co-ordinates provided")
    sys.exit()

if (frdata == ''):
    if (ERdata == ''):
        print("\t- No Epoch-Range datafile provided")
        sys.exit()
    elif (ERdata != ''):
        print('\t+ Epoch-Range datafile is ', ERdata)
        dataf = ERdata

        if (INmjd == 0.0):
            print('\tModified Julian Day not provided')
            sys.exit()

        if (autoCPF) & (SATtarget_name == ''):
            print('\t- No Satellite name provided, cannot auto-fetch CPF file ')
            setupWarningList.append("No Satellite name provided, cannot auto-fetch CPF file ")
            autoCPF = False

        if (METin == ''):
            print('\t- No meteorological data provided')
            setupWarningList.append("No meteorological data provided")
            METap = False
        else:
            print('\t+ Met file is ', METin)

        if (CALin == ''):
            print('\t- No system delay calibration data provided')
            setupWarningList.append("No system delay calibration data provided")
        else:
            print('\t+ Cal file is ', CALin)

    if (frate == 0.0):
        print('\t- *** Laser Fire rate not provided! *** ')
        setupWarningList.append("*** Laser Fire rate not provided! ***")
else:
    print('\t+ FRD file is', frdata)
    dataf = frdata

if (autoCPF == False) & (CPFin == ''):
    print('\t- CPF file not provided')
    sys.exit()

if (autoCPF) & (CPFin != ''):
    print("\t+ CPF file provided as " + CPFin + " not fetching CPF")
    autoCPF = False
elif (CPFin != ''):
    print('\t+ CPF Pred is', CPFin)
elif (pstr != ''):
    print('\t+ CPF provider is', pstr)
if (autoCPF):
    print('\t+ Attempt to fetch corresponding CPF from EDC Data Cetre')

if (STAT_id != ''):
    print('\t+ SLR Station is', STAT_id)
print('\t+ Station Latitude, Longitude and Height: ', STAT_LAT, STAT_LONG, STAT_HEI)

if (NPbin_length == -1.0):
    print('\t- Normal Point bin length NOT defined, using default 30 seconds')
    setupWarningList.append("Normal Point bin length NOT defined, using default 30 seconds")
    NPbin_length = 30.0

if (minNPn == -1.0):
    print('\t- Minimum number of observations for a Normal Point NOT defined, using default 30')
    setupWarningList.append("Minimum number of observations for a Normal Point NOT defined, using default 30")
    minNPn = 30

if (clipLEHM & clipsigma):
    print('\t- 3*sigma clipping and LEHM clipping cannot be applied together')
    sys.exit()
elif (clipLEHM):
    print('\t+ Apply clipping from LEHM')
elif (clipLEHM):
    print('\t+ Apply 3*sigma clipping')

if (NPout):
    print('\t+ Output normal points to file normalp.dat in CRD format')

if (RRout):
    print('\t+ Output range residuals to file resids.dat')

if (SLVout):
    print('\t+ Output obit solve parameters to file solvep.out')

if (Unfilter):
    print('\t+ Include unfiltered range measurements')

if (displayRES):
    print('\t+ Produce plot and save and display')
elif (plotRES):
    print('\t+ Produce plot and save but no display')
if (savepassname != ''):
    print('\t+ Save plot as ', savepassname + '.png')

nu = 13  # lsq size
sol = 299792458.0  # Speed of Light m/s
sw = 2.0e-8  # weighting applied to residual fitting

dae = 6.378137000  # PARAMETERS OF SPHEROID
df = 298.2570

# calculate Station lat, long coordinates in radians
STAT_LONGrad = STAT_LONG * 2 * np.pi / 360.0
STAT_LATrad = STAT_LAT * 2 * np.pi / 360.0
STAT_HEI_Mm = STAT_HEI * 1e-6

read1st = True
while (loop):
    if (FRDloop == False):
        loop = False

    pep = 0.0
    pepm = 0.0
    pepc = 0.0
    Mmep = list()
    pressure = list()
    TK = list()
    humid = list()
    Depc = list()
    Ddatet = list()
    Dmep = list()
    Drng = list()
    Cmep = list()
    Crng = list()
    STsel = False
    SDapplied = True
    runWarningList = []

    if (STAT_id == ''):
        STsel = True

    if frdata != '':
        # If the data file provided is a full rate data file in CRD format read in the data header H4,
        # the met data entries and the Epoch-Range data.
        # Open frd file and read met records and ranges
        print('\n -- Read FRD file ... ')
        try:
            fid = open(frdata)
        except IOError as e:
            print("\tI/O ERROR({0}): {1}   {2}".format(e.errno, e.strerror, frdata))

        if (read1st):
            h2i = list()
            h2l = list()
            h4l = list()
            c10 = 0
            Fcount = list()
            for l, line in enumerate(fid):
                if "h2" in line or "H2" in line:
                    if c10 > 0:
                        Fcount.append(c10)
                    c10 = 0
                    h2i.append(l)
                    h2l.append(line.split()[2])
                if "h4" in line or "H4" in line:
                    h4l.append(line.strip())
                if line[0:2] == '10':
                    c10 = c10 + 1
            Fcount.append(c10)
            read1st = False

        lnpass = 1
        h2i = np.array(h2i)
        h2l = np.array(h2l)

        numpass = np.sum(h2l == STAT_id)

        print(h2l)
        print(STAT_id)

        selpass = np.where(h2l == STAT_id)[0]
        if (numpass == 0):
            print(" EXIT: No data for station", STAT_id, "in FRD file")
            sys.exit()
        elif (numpass == 1):
            print('\n FRD file contains only one pass for station', STAT_id)
            lnpass = h2i[selpass[0]]
            loop = False
            pass
        else:
            ipass = -1
            # sel=np.where(h2l == STAT_id)[0]
            h4l = np.array(h4l)
            print('\t', 'Index', '\t', 'Station Name', '    ', 'Num Records', '       ', 'H4 Start/End Entry')
            for i, s in enumerate(selpass):
                print('\t', i, '\t', STAT_name, '\t', "{:8d}".format(Fcount[s]), ' \t', h4l[s])
            print('\n FRD file contains', numpass, 'passes for station', STAT_id, '\t\t\t(q to quit)')
            while ((ipass < 0) | (ipass >= numpass)):
                try:
                    rawi = input('Enter pass number: ')  # python3 specific
                    ipass = int(rawi)
                except:
                    if (rawi == 'q'):
                        sys.exit()
                    ipass = -1
            lnpass = h2i[selpass[ipass]]

        fid.seek(0)
        line = fid.readline()
        if (line.split()[0] != 'H1') & (line.split()[0] != 'h1'):
            print(" ERROR: FRD input file read error")
            sys.exit()

        print('\n -- Read FRD file for epochs, ranges and meteorological data... ')
        fid.seek(0)
        for i, line in enumerate(fid):
            a = line.split()

            if (a[0] == 'H1') | (a[0] == 'h1'):
                CRDversion = a[2]

            if (a[0] == 'H2') | (a[0] == 'h2'):
                if ((STAT_id == a[1]) | (STAT_id == a[2]) & (i == lnpass)):
                    STsel = True
            if STsel:
                if (a[0] == 'H3') | (a[0] == 'h3'):
                    Hsat = a[1]
                    SATtarget_name = Hsat
                elif (a[0] == 'H4') | (a[0] == 'h4'):
                    Hyr = a[2]
                    Hmon = "{:02d}".format(int(a[3]))
                    Hd = "{:02d}".format(int(a[4]))
                    mjd1 = cal2mjd(int(a[2]), int(a[3]), int(a[4])) + (
                                float(a[5]) + float(a[6]) / 60.0 + float(a[7]) / 3600.0) / 24.0
                    mjd2 = cal2mjd(int(a[8]), int(a[9]), int(a[10])) + (
                                float(a[11]) + float(a[12]) / 60.0 + float(a[13]) / 3600.0) / 24.0
                    INmjd = cal2mjd(int(a[2]), int(a[3]), int(a[4]))
                    mjdm = INmjd
                    mjdc = INmjd
                    c = mjd2cal(INmjd)

                    if (a[18] == '0'):
                        SDapplied = False
                        print("\n -- System Delay Calibration not applied.  Will be applied")
                        runWarningList.append("System Delay Calibration was applied to ranges")
                    else:
                        print("\n -- System Delay Calibration already applied")
                elif (a[0] == "C1") | (a[0] == 'c1'):  # read from C1 entry
                    frate = float(a[5])
                elif a[0] == '10':
                    if (Unfilter) | (a[5] == '2' or a[5] == '0'): # JRM Mod. Now we send data as unknown Fixme
                        ep = np.double(a[1]) / 86400.0
                        if (ep < pep):
                            INmjd = INmjd + 1.0
                            c = mjd2cal(INmjd)
                        pep = ep
                        Dmep.append(INmjd + ep)
                        Depc.append(np.double(a[1]))
                        Drng.append(np.double(a[2]) * 1.0e12)
                        cep = np.double(a[1])
                        Ddatet.append(dt.datetime(c[0], c[1], c[2]) + dt.timedelta(seconds=cep))
                elif a[0] == '20':
                    epm = np.double(a[1]) / 86400.0
                    # Check if first met entry if from previous day
                    if (pepm == 0.0) & (epm - np.mod(mjd1, 1) > 0.5):
                        print("\n -- Met dataset begins on previous day")
                        runWarningList.append("Met dataset begins on previous day")
                        mjdm = mjdm - 1.0

                    # Detect day change in met entries
                    if (epm < pepm):
                        mjdm = mjdm + 1.0

                    pepm = epm
                    Mmep.append(mjdm + epm)
                    pressure.append(np.double(a[2]))
                    TK.append(np.double(a[3]))
                    humid.append(np.double(a[4]))
                elif a[0] == '40':
                    epc = np.double(a[1]) / 86400.0
                    Cmep.append(INmjd + epc)
                    Crng.append(np.double(a[7]))

                elif (a[0] == 'H8') | (a[0] == 'h8'):
                    break

        fid.close()

    else:
        print('\n -- Read raw epoch-range data file ... ')
        c = mjd2cal(INmjd)
        Hyr = str(c[0])
        Hmon = "{:02d}".format(int(c[1]))
        Hd = "{:02d}".format(int(c[2]))
        Hsat = SATtarget_name
        fid = open(ERdata)
        for line in fid:
            a = line.split()
            ep = np.double(a[0]) / 86400.0
            if (ep < pep):
                INmjd = INmjd + 1.0
                c = mjd2cal(INmjd)
            pep = ep
            Dmep.append(INmjd + np.double(a[0]) / 86400.0)
            Depc.append(np.double(a[0]))
            Drng.append(int(float(a[1]) * 1.0e12))
            cep = np.double(a[0])
            Ddatet.append(dt.datetime(c[0], c[1], c[2]) + dt.timedelta(seconds=cep))

        fid.close()
        # mjd1 = 0.2*np.floor(5.0*np.min(Dmep))
        # mjd2 = 0.0*np.ceil(5.0*np.max(Dmep))
        mjd1 = np.min(Dmep)
        mjd2 = np.max(Dmep)

        if (METin != ''):
            fid = open(METin)
            for line in fid:
                a = line.split()
                Mmep.append(np.double(a[0]) + np.double(a[1]) / 86400.0)
                pressure.append(np.double(a[2]))
                TK.append(np.double(a[3]))
                humid.append(np.double(a[4]))
            fid.close()

        if (CALin != ''):
            fid = open(CALin)
            for line in fid:
                a = line.split()
                Cmep.append(np.double(a[0]) + np.double(a[1]) / 86400.0)
                Crng.append(np.double(a[2]))
            fid.close()
            SDapplied = False

    mean_Dmep = np.mean(Dmep)  # mean data Depc

    if (mjd2 < mjd1):  # Correct H4 record
        mjd2 = mjd1 + (cep.argmax() - cep.argmin()) / 86400.0

    if (SDapplied == False):
        if (len(Cmep) > 1):
            IntpC = interpolate.interp1d(Cmep, Crng, kind='linear', bounds_error=False, fill_value=(Crng[0], Crng[-1]))
            Drng = Drng - IntpC(Dmep)
        else:
            Drng = Drng - Crng[0]

    if np.size(Dmep) == 0:
        print(" No Epoch-Range data loaded, quitting...", STsel)
        sys.exit()

    if (frdata != '') | (METin != ''):
        # Set up linear interpolation Python function for the met data entries
        print('\n -- Interpolate meteorological records ... ')

        if np.size(Mmep) > 1:
            IntpP = interpolate.interp1d(Mmep, pressure, kind='linear', bounds_error=False,
                                         fill_value=(pressure[0], pressure[-1]))
            IntpT = interpolate.interp1d(Mmep, TK, kind='linear', bounds_error=False, fill_value=(TK[0], TK[-1]))
            IntpH = interpolate.interp1d(Mmep, humid, kind='linear', bounds_error=False,
                                         fill_value=(humid[0], humid[-1]))

            # Produce a met value for each data Depc using the interpolation functions
            PRESSURE = IntpP(Dmep)
            TEMP = IntpT(Dmep)
            HUM = IntpH(Dmep)
        else:
            PRESSURE = pressure[0]
            TEMP = TK[0]
            HUM = humid[0]

        # Test if met data epochs are suitable
        iMax = abs(np.max(Dmep) - (Mmep)).argsort()[0]
        iMin = abs(np.min(Dmep) - (Mmep)).argsort()[0]
        if ((abs(np.max(Dmep) - Mmep[iMax]) < 0.5 / 24.0) & (abs(np.min(Dmep) - Mmep[iMin]) < 1.5 / 24.0)):
            METap = True

    # If flag -A is set, use the FRD filename to automatically retrieve the corresponding CPF prediction file
    if autoCPF:
        crefep = Dmep[0]
        if np.mod(crefep, 1) < 0.0025:
            Hd = "{:02d}".format(int(Hd) - 1)
            runWarningList.append("Pass begins soon after midnight, prediction from day earlier used")
            print('\n -- Pass begins soon after midnight, searching for prediction from day earlier')

        print('\n -- Fetching CPF prediction file corresponding to FRD file ... ')
        os.system("rm CPF.list")
        Hsat = Hsat.lower()
        if pstr != '':
            # cpf_s=Hsat+'_cpf_'+frdata.split('_')[3][2:]+'_*.'+pstr
            cpf_s = Hsat + '_cpf_' + Hyr[-2:] + Hmon + Hd + '_*.' + pstr
        else:
            # cpf_s=Hsat+'_cpf_'+frdata.split('_')[3][2:]+'_*'
            cpf_s = Hsat + '_cpf_' + Hyr[-2:] + Hmon + Hd + '_*'

        # save image to file. Use ipass index if multi pass FR data file used
        cfol = 'CPF/'
        if not os.path.exists(cfol):
            os.makedirs(cfol)

        cmd = "wget -Nq ftp://edc.dgfi.tum.de/pub/slr/cpf_predicts/" + Hyr + '/' + Hsat + '/' + cpf_s + " -P CPF"
        os.system(cmd);
        print(cmd)
        dwl = int(subprocess.Popen('ls ' + cfol + cpf_s + ' | wc -l', shell=True, stdout=subprocess.PIPE).communicate()[
                      0].strip())
        cmd = "ls " + cfol + cpf_s + "> CPF.list"
        os.system(cmd)
        cpf_lfi = open("CPF.list")
        for c, cfile in enumerate(cpf_lfi):
            cid = open(cfile.strip())
            for cl in cid:
                if cl[0:2] == '10':
                    a = cl.split()
                    cpf_iep = float(a[2]) + float(a[3]) / 86400.0
                    break

            if (crefep - cpf_iep < 0.002):
                dwl = dwl - 1

            cid.close()
        cpf_lfi.close()

        if (dwl == 0):
            runWarningList.append("No prediction found for same day from provider '" + pstr + "'. Alternative used")
            print('\n -- Attempting to find alternative CPF prediction file corresponding to FRD file ...')
            frdt = dt.date(int(Hyr), int(Hmon), int(Hd))
            os.system("rm .listing")
            cmd = "wget --spider --no-remove-listing ftp://edc.dgfi.tum.de/pub/slr/cpf_predicts/" + Hyr + '/' + Hsat + '/'
            os.system(cmd)
            cpf_listing = open(".listing")

            # mindt= dt.date(1,1,1)
            for entry in cpf_listing:
                a = entry.split()
                if (a[-1][0] != '.'):
                    cpfdate = a[-1].split('_')[-2]
                    cpfdt = dt.date(2000 + int(cpfdate[0:2]), int(cpfdate[2:4]), int(cpfdate[4:6]))
                    if (frdt - cpfdt < dt.timedelta(days=2)) & (frdt >= cpfdt):
                        selpred = a[-1]
                        cmd = "wget -Nq ftp://edc.dgfi.tum.de/pub/slr/cpf_predicts/" + Hyr + '/' + Hsat + '/' + selpred + " -P CPF"
                        os.system(cmd);
                        cmd = "ls " + cfol + selpred + ">> CPF.list"
                        os.system(cmd)

            cpf_listing.close()

        numprov = 0
        iprov = 0
        cpf_l = []
        cpf_lfi = open("CPF.list")
        for c, cfile in enumerate(cpf_lfi):
            cid = open(cfile.strip())
            for cl in cid:
                if cl[0:2] == '10':
                    a = cl.split()
                    cpf_iep = float(a[2]) + float(a[3]) / 86400.0
                    break
            cid.close()

            if (crefep - cpf_iep > 0.002):
                cpf_l.append(cfile.strip())
                numprov = numprov + 1

        if numprov > 1:
            if pstr != '':
                iprov = numprov - 1
            else:
                print('')
                for c, cfile in enumerate(cpf_l):
                    print('\t', c, '\t', cfile)

                iprov = -1
                print('')
                while ((iprov < 0) | (iprov >= numprov)):
                    try:
                        iprov = int(input('  Select prediction provider: '))
                    except:
                        iprov = -1

        if (numprov == 0):
            print("No CPF prediction found ")
            sys.exit()

        CPFin = cpf_l[iprov]
        cpf_lfi.close()

    # Read CPF Prediction File in the Depc, X, Y, Z lists and produce interpolation functions
    if CPFin == '':
        print(" ERROR: No CPF file")
        sys.exit()
    else:
        print('\n -- Read CPF prediction file:', CPFin)
        cpf_fid = open(CPFin)

        cpfEP = list()
        cpfX = list()
        cpfY = list()
        cpfZ = list()

        mep2 = 0.0
        stp = 0.0
        for line in cpf_fid:
            a = line.split()
            if a[0] == "10":
                mep = np.double(a[2]) + np.double(a[3]) / 86400.0
                if ((stp == 0.0) & (mep2 != 0.0)):
                    stp = mep - mep2
                mep2 = mep
                if (mep >= (mjd1 - 1.5 / 24.0) - 3.0 * stp) & (mep <= (mjd2 + 1.5 / 24.0) + 4.0 * stp):
                    cpfEP.append(mep)
                    cpfX.append(np.double(a[5]))
                    cpfY.append(np.double(a[6]))
                    cpfZ.append(np.double(a[7]))

        cpf_fid.close()

        kd = 15
        if (np.size(cpfEP) <= kd):
            kd = np.size(cpfEP) - 1

        # Set up linear interpolation Python function for CPF X, Y and Z components
        try:
            cpf_IntpX = interpolate.interp1d(cpfEP, cpfX, kind=kd)
            cpf_IntpY = interpolate.interp1d(cpfEP, cpfY, kind=kd)
            cpf_IntpZ = interpolate.interp1d(cpfEP, cpfZ, kind=kd)
        except:
            kd = 9  # int(0.5*kd)+1
            cpf_IntpX = interpolate.interp1d(cpfEP, cpfX, kind=kd)
            cpf_IntpY = interpolate.interp1d(cpfEP, cpfY, kind=kd)
            cpf_IntpZ = interpolate.interp1d(cpfEP, cpfZ, kind=kd)

    geoR = np.sqrt(cpf_IntpX(mean_Dmep) ** 2 + cpf_IntpY(mean_Dmep) ** 2 + cpf_IntpZ(mean_Dmep) ** 2)

    print('\n -- Begin orbit adjustment to fit range data')
    neps = len(Depc)
    nmet = len(Mmep)

    # Calculate mid-pass time in seconds. This time is used as origin of time-dependent unknowns.
    if (Depc[-1] > Depc[0]):
        dtobc = (Depc[-1] + Depc[0]) / 2.0
    else:
        dtobc = 0.0

    # generate arrays
    gvs = np.zeros(3)
    cv = np.zeros(nu)
    rhs = numpy.array(np.zeros(nu), order='F')
    rd = numpy.array(np.zeros([nu, nu]), order='F')
    rf = np.zeros(nu)
    s = np.zeros(nu)

    # zero variables
    itr = 0
    itrm = 24
    alnc = 0.0
    acrc = 0.0
    radc = 0.0
    alndc = 0.0  # Accumulated Satellite orbital time bias
    acrdc = 0.0  # Accumulated Rate of time bias
    raddc = 0.0  # Accumulated Satellite radial error
    alnddc = 0.0  # Accumulated Rate of radial error
    acrddc = 0.0  # Accumulated Acceleration of time bias
    radddc = 0.0  # Accumulated Acceleration of radial error

    alnd = 0.0
    rdld = 0.0
    alndd = 0.0
    rdldd = 0.0
    saln = 0.0
    sacr = 0.0
    srdl = 0.0

    salnd = 0.0
    sacrd = 0.0
    srdld = 0.0
    salndd = 0.0
    sacrdd = 0.0
    srdldd = 0.0

    ierr = 0

    sigt = 0.1 / 8.64e7
    sigr = 0.01 / 1.0e6
    sigtt = 0.1 / 8.64e7
    sigrr = 0.01 / 1.0e6

    oldrms = 1000.0

    #     compute n+h and (n*b*b/a*a)+h given geodetic co-ordinates and spheroidal parameters a and f.

    db = dae * (1.0 - 1.0 / df)
    dn = dae / (np.sqrt(
        np.cos(STAT_LATrad) * np.cos(STAT_LATrad) + (db * db * np.sin(STAT_LATrad) * np.sin(STAT_LATrad)) / (
                    dae * dae)))  # Earth Radius at Station Lat/LONG
    dnh = dn + STAT_HEI_Mm
    dnab = (dn * db * db) / (dae * dae)
    dnabh = dnab + STAT_HEI_Mm

    # Calculate the geocentric geocentric co-ordinates of SLR telescope from
    # wrt true equator of date and x-axis same as that of ephemeris

    STAT_X = dnh * np.cos(STAT_LATrad) * np.cos(STAT_LONGrad)
    STAT_Y = dnh * np.cos(STAT_LATrad) * np.sin(STAT_LONGrad)
    STAT_Z = dnabh * np.sin(STAT_LATrad)

    ql = list()
    qm = list()
    qn = list()

    ddr = list()

    dxi = list()
    dyi = list()
    dzi = list()

    dvx = list()
    dvy = list()
    dvz = list()

    t = list()
    p = list()
    h = list()

    cpfR = np.sqrt((cpf_IntpX(Dmep) - STAT_X * 1e6) ** 2 + (cpf_IntpY(Dmep) - STAT_Y * 1e6) ** 2 + (
                cpf_IntpZ(Dmep) - STAT_Z * 1e6) ** 2)  # Range from SLR Station to satellite in metres

    dkt = Dmep + (cpfR / sol) / 86400.0  # Time of laser light arrival at satellite
    dkt1 = dkt - 0.5 / 86400.0  # Epoch - 0.5 seconds
    dkt2 = dkt + 0.5 / 86400.0  # Epoch + 0.5 seconds

    cX = cpf_IntpX(dkt) * 1e-6  # X component of satellite CPF prediction in megametres
    cY = cpf_IntpY(dkt) * 1e-6  # Y component of satellite CPF prediction in megametres
    cZ = cpf_IntpZ(dkt) * 1e-6  # Z component of satellite CPF prediction in megametres

    vX = (cpf_IntpX(dkt2) - cpf_IntpX(dkt1)) * 1e-6 * 86400.0  # X velocity component in megametres/day
    vY = (cpf_IntpY(dkt2) - cpf_IntpY(dkt1)) * 1e-6 * 86400.0  # Y velocity component in megametres/day
    vZ = (cpf_IntpZ(dkt2) - cpf_IntpZ(dkt1)) * 1e-6 * 86400.0  # Z velocity component in megametres/day

    ddr = np.sqrt(cX ** 2 + cY ** 2 + cZ ** 2)  # Radial distance to satellite from geo-centre
    dvel = np.sqrt(vX ** 2 + vY ** 2 + vZ ** 2)  # Velocity magnitude
    rv = ddr * dvel

    ql = (cY * vZ - cZ * vY) / rv  # X component of across track from cross products
    qm = (cZ * vX - cX * vZ) / rv  # Y component of across track
    qn = (cX * vY - cY * vX) / rv  # Z component of across track

    Depc = np.array(Depc)
    Drng = np.array(Drng)
    Ddatet = np.array(Ddatet)
    Dmep = np.array(Dmep)

    dxi = np.array(cX)
    dyi = np.array(cY)
    dzi = np.array(cZ)

    dvx = np.array(vX)
    dvy = np.array(vY)
    dvz = np.array(vZ)

    ddr = np.array(ddr)
    ql = np.array(ql)
    qm = np.array(qm)
    qn = np.array(qn)

    zdum = np.zeros(neps)

    rej2 = 1.e10  # set initial large rejection level
    rej3 = 1.e10
    rej5 = 1.e10
    rmsa = 0.0

    itr_fin = False
    # Iteration loop in which the orbit correction parameters are adjusted to give a best fit to the residuals.
    while (itr <= itrm) & (itr_fin == False):

        #   print ( '\rloop {0}\r'.format(itr),)
        itr = itr + 1  # Iteration number

        ssr = 0.0
        nr = 0
        oldrms = rmsa

        rhs = np.zeros([nu], dtype= np.longdouble)
        cv = np.zeros([nu], dtype= np.longdouble)
        rd = numpy.array(np.zeros([nu, nu], dtype= np.longdouble), order='F')

        # apply along-track, across-track and radial corrections to satellite geocentric coordinates. the corrections
        # have been determined from previous iteration, and accumulated values are stored in variables
        # alnc (in days), acrc and radc (in megametres)

        tp = (Depc - dtobc) / 60.0  # Time measured from mid-time of pass
        if (Depc[-1] < Depc[0]):
            sel = np.where(Depc > Depc[-1])
            tp[sel] = (Depc[sel] - 86400.0) / 60.0

        # Evaluate constant terms + time rates of change
        # argument minutes of time, measured from mid-time of pass

        al = alnc + alndc * tp + alnddc * tp * tp  # Along track correction from accumulated values, rates and accelerations
        ac = acrc + acrdc * tp + acrddc * tp * tp  # Across track correction
        ra = radc + raddc * tp + radddc * tp * tp  # Radial correction

        # Update XYZ coordinates from the across track, long track and radial corrections
        dx = dxi + dvx * al + ql * ac + (dxi * ra / ddr)
        dy = dyi + dvy * al + qm * ac + (dyi * ra / ddr)
        dz = dzi + dvz * al + qn * ac + (dzi * ra / ddr)

        dxt = dx - STAT_X  # X component difference between satellite and station in megametres
        dyt = dy - STAT_Y  # Y component
        dzt = dz - STAT_Z  # Z component
        dr = np.sqrt(dxt * dxt + dyt * dyt + dzt * dzt)  # Range from station to satellite
        drc = dr * 2.0  # 2 way range in megametres

        # Calculate the telescope elevation angle for input to the refraction delay model from the satellite altitude
        # relative to geodetic zenith.

        gvs[0] = np.cos(STAT_LATrad) * np.cos(STAT_LONGrad)  # Station X unit vector
        gvs[1] = np.cos(STAT_LATrad) * np.sin(STAT_LONGrad)  # Station Y unit vector
        gvs[2] = np.sin(STAT_LATrad)  # Station Z unit vector
        dstn = np.sqrt(gvs[0] * gvs[0] + gvs[1] * gvs[1] + gvs[2] * gvs[2])  # Normalise the unit vectors
        czd = (dxt * gvs[0] + dyt * gvs[1] + dzt * gvs[2]) / (
                    dr * dstn)  # Zenith height component of SAT->STAT vector / vector range
        altc = np.arcsin(czd) * 360.0 / (2.0 * np.pi)  # inverse sin() to give elevation

        if (itr <= itrm):
            #  Compute partial derivatives. First, range wrt along-track, across-track and radial errors in the predicted ephemeris.
            drdal = (dvx * dxt + dvy * dyt + dvz * dzt) / dr
            drdac = (ql * dxt + qm * dyt + qn * dzt) / dr
            drdrd = (dx * dxt + dy * dyt + dz * dzt) / (dr * ddr)

        # Time rates of change of these partials are computed by multiplying the above constant quantities by time in
        # minutes from mid-pass time (tp). For accelerations, multiply by tp*tp.
        # These multiplications are carried out below,when the equation of condition vector 'cv' is set up.

        cv = [drdal, drdac, drdrd, drdal * tp, drdac * tp, drdrd * tp, drdal * tp * tp, drdac * tp * tp,
              drdrd * tp * tp, zdum, zdum, zdum, zdum]

        # Introduce weights. Set SE of a single observation (sw) to 2 cm
        for j in range(9):
            cv[j] = cv[j] / sw

        # Compute refraction delay and apply to predicted 2-way range , using the Marini and Murray model.
        if (nmet > 0):
            refr = refmm(PRESSURE, TEMP, HUM, altc, 0.532, STAT_LATrad, STAT_HEI_Mm)
            delr = refr * 1.0e-6
            drc = drc + delr

        drc = (drc / (sol * 1e-6)) * 1.0e9  # convert computed range to nsecs (2 way)
        tresid = (Drng * 1.0e-3 - drc) / 2.0  # 1 way observational o-c in nsecs
        dresid = tresid * sol * 1e-6 * 1.0e-9  # o-c in Mm for solution and rejection test
        dresid = dresid / sw  # weight residual

        aresid = abs(dresid)
        #  Use data within 3.0*rms for determining rms. Use data within 2.0*rms for solution. Output data within 5.0*rms

        Ssel = np.where(aresid < rej2)[0]
        if (len(Ssel) > 0.98 * len(aresid)):  # Ensure central data is selected for orbit
            Ssel = aresid.argsort()[:-int(0.02 * len(aresid)) - 1]
        rmsb = np.std(dresid[Ssel])

        Rsel = np.where(aresid < rej3)[0]
        if (len(Rsel) > 0.95 * len(aresid)):  # Ensure central data is selected for RMS
            Rsel = aresid.argsort()[:-int(0.05 * len(aresid)) - 1]
        rms3 = np.std(dresid[Rsel])
        if (itr == 1):
            print("\n\t  #     pts          rms2          rms3          rmsa")

        print("\t{:3d} {:8d}   {:11.3f}   {:11.3f}   {:11.3f}".format(itr, np.size(Ssel), 1e9 * rmsb * sw,
                                                                      1e9 * rms3 * sw, 1000.0 * rmsa * sw))

        rej5 = 5.0 * rms3
        rej3 = 3.0 * rms3
        rej2 = 2.5 * rms3

        ssr = np.sum(dresid[Ssel] * dresid[Ssel], dtype=np.longdouble)  # Sum of residual squares
        nr = np.size(Ssel)  # number of residuals

        if (itr == 1):  # Plot residuals
            pltresx1 = Depc
            pltresy1 = tresid
        pltres = tresid * 1.0e3

        # Form normal eqns.
        for j in range(nu):
            rhs[j] = np.sum(cv[j][Ssel] * dresid[Ssel], dtype=np.longdouble)
            for k in range(nu):
                rd[k, j] = np.sum(cv[j][Ssel] * cv[k][Ssel], dtype=np.longdouble)

        # Apply A-PRIORI standard errors to unknowns tdot,rdot,tddot,rddot values are stored in data statement.
        if (itr < itrm):
            rd[3, 3] = rd[3, 3] + (1.0 / sigt) ** 2
            rd[5, 5] = rd[5, 5] + (1.0 / sigr) ** 2
            rd[6, 6] = rd[6, 6] + (1.0 / sigtt) ** 2
            rd[8, 8] = rd[8, 8] + (1.0 / sigrr) ** 2

            rhs[3] = rhs[3] + (1.0 / sigt) * alnd
            rhs[5] = rhs[5] + (1.0 / sigr) * rdld
            rhs[6] = rhs[6] + (1.0 / sigtt) * alndd
            rhs[8] = rhs[8] + (1.0 / sigrr) * rdldd

            nus = [1, 4, 7]  # Suppress across track unknowns
            rd[nus, :] = 0.0
            rd[:, nus] = 0.0
            rd[nus, nus] = 1.0
            rhs[nus] = 0.0

            ins = 3

            nus = [9, 10, 11, 12]  # Suppress pulse-dependent mean values for all but penultimate iteration.
            rd[nus, :] = 0.0
            rd[:, nus] = 0.0
            rd[nus, nus] = 1.0
            rhs[nus] = 0.0

            # Carry out least squares solution
            ierr, rd = dchols(rd, nu)  # invert normal matrix
            if (ierr != 0):
                print("FAILED to invert normal matrix - quit", ierr)
                sys.exit()

            for i in range(nu):
                rf[i] = 0.0

            # Form solution vector, rf.
            for i in range(nu):
                for j in range(nu):
                    rf[i] = rf[i] + rd[i, j] * rhs[j]

            # Form sum of squares after solution.
            rra = ssr
            for i in range(nu):
                rra = rra - rf[i] * rhs[i]

            if (rra < 0.0):
                rra = 0.0

            rmsa = np.sqrt(rra / nr) * 1.0e6
            seuw = rra / (1.0 * (nr - nu + ins))

            # Form vector of standard errors, s
            for i in range(nu):
                s[i] = 0.0
                if (rhs[i] != 0.0):
                    s[i] = np.sqrt(rd[i, i] * seuw)

            if (itr < (itrm - 1)):
                aln = rf[0]  # along track corrections
                saln = s[0] * 8.64e7

                acr = rf[1]  # across track corrections
                sacr = s[1] * 1.0e6

                rdl = rf[2]  # radial correction
                srdl = s[2] * 1.0e6

                # Get corrections to rates of change of those parameters and their accelerations.

                alnd = rf[3]
                acrd = rf[4]
                rdld = rf[5]
                salnd = s[3] * 8.64e7
                sacrd = s[4] * 1.0e6
                srdld = s[5] * 1.0e6

                alndd = rf[6]
                acrdd = rf[7]
                rdldd = rf[8]
                salndd = s[6] * 8.64e7
                sacrdd = s[7] * 1.0e6
                srdldd = s[8] * 1.0e6

                # Accumulate corrections during iteration

                alnc = alnc + aln
                acrc = acrc + acr
                radc = radc + rdl
                alndc = alndc + alnd
                acrdc = acrdc + acrd
                raddc = raddc + rdld
                alnddc = alnddc + alndd
                acrddc = acrddc + acrdd
                radddc = radddc + rdldd

        if (abs(oldrms - rmsa) * sw < 0.00001) & (itr > 8):
            if not itr_fin:
                itrm = itr + 2
                itr_fin = True

    print("\n\tSatellite orbital time bias (ms)  ", "{:9.3f}".format(alnc * 8.64e7), "{:7.3f}".format(saln))
    print("\tSatellite radial error (m)        ", "{:9.3f}".format(radc * 1.0e6), "{:7.3f}".format(srdl))
    print("\tRate of time bias (ms/minute)     ", "{:9.3f}".format(alndc * 8.64e7), "{:7.3f}".format(salnd))
    print("\tRate of radial error (m/minute)   ", "{:9.3f}".format(raddc * 1.0e6), "{:7.3f}".format(srdld))
    print("\tAcceleration of time bias         ", "{:9.3f}".format(alnddc * 8.64e7), "{:7.3f}".format(salndd))
    print("\tAcceleration of radial error      ", "{:9.3f}".format(radddc * 1.0e6), "{:7.3f}".format(srdldd))

    if (abs(alnc * 8.64e7) > 100.0):
        runWarningList.append("Large Time Bias required " + "{:9.3f}".format(alnc * 8.64e7) + " ms")
    elif (abs(alnc * 8.64e7) > 10.0):
        runWarningList.append("Time Bias required " + "{:9.3f}".format(alnc * 8.64e7) + " ms")

    if (abs(radc * 1.0e6) > 100.0):
        runWarningList.append("Large Range Bias required " + "{:9.3f}".format(radc * 1.0e6) + " m")
    elif (abs(radc * 1.0e6) > 10.0):
        runWarningList.append("Range Bias required " + "{:9.3f}".format(radc * 1.0e6) + " m")

    if (SLVout):
        # write solve parameters to file
        fileslv = open("solvep.out", "w")
        fileslv.write("{:9.3f}".format(alnc * 8.64e7) + "  " + "{:7.3f}".format(saln) + "\n")
        fileslv.write("{:9.3f}".format(radc * 1.0e6) + "  " + "{:7.3f}".format(srdl) + "\n")
        fileslv.write("{:9.3f}".format(alndc * 8.64e7) + "  " + "{:7.3f}".format(salnd) + "\n")
        fileslv.write("{:9.3f}".format(raddc * 1.0e6) + "  " + "{:7.3f}".format(srdld) + "\n")
        fileslv.write("{:9.3f}".format(alnddc * 8.64e7) + "  " + "{:7.3f}".format(salndd) + "\n")
        fileslv.write("{:9.3f}".format(radddc * 1.0e6) + "  " + "{:7.3f}".format(srdldd) + "\n")
        fileslv.close()

    presid = tresid * 1e3
    cRMS = np.std(presid)
    aresid = abs(presid)
    cLEN = np.max(presid) - np.min(presid)

    if (RRout):
        # write range residuals to a file
        fileo = open("resids.dat", "w")
        for i, ep in enumerate(Depc):
            fileo.write("{:18.7f}".format(ep) + " " + "{:14.12f}".format(1e-12 * Drng[i]) + " " + "{:18.12f}".format(
                presid[i] * 1e-3) + " 1 " + "{:18.12f}".format(ep) + "\n")
        fileo.close()

    iRMS = cRMS
    prevRMS = 0.0

    psecbin = 2
    if (iRMS > 100.0) | (np.size(presid) < 1500):
        psecbin = 4

    pmin = np.min(presid)
    pmax = np.max(presid)
    nbins = int((pmax - pmin) / psecbin)

    i = 0
    while (nbins > 50000):
        i = i + 1
        pmin = np.min(presid[abs(presid).argsort()[:-i]])
        pmax = np.max(presid[abs(presid).argsort()[:-i]])
        nbins = int((pmax - pmin) / psecbin)
        print(i, nbins, pmax, pmin, psecbin)
    if nbins < 100:
        nbins = 100

    if nbins > 5000:
        print("Large number of histogram bins required.  Plot Processing slow...    " + str(nbins) + " bins.")
        runWarningList.append(
            "Large number of histogram bins required.  Plot Processing slow...    " + str(nbins) + " bins.")

    [hbins, hstep] = np.linspace(pmin, pmax, nbins, retstep=True)


    # Define model function to be used to fit to the data above:
    def gauss(x, *p):
        A, mu, sigma = p
        return A * np.exp(-(x - mu) ** 2 / (2. * sigma ** 2))


    LEHM = 0.0
    PEAK = 0.0

    if (clipsigma):
        print('\n -- Clipping at N-sigma')
        iRMS = cRMS
        prevRMS = 0.0
        imean = 0.0
        citr = 0
        while abs(prevRMS - iRMS) > 0.0030:
            citr = citr + 1
            Osel = np.where(abs(presid - imean) < cfactor * iRMS)[0]
            prevRMS = iRMS
            iRMS = np.std(presid[Osel])
            imean = np.mean(presid[Osel])
            print(len(Osel), iRMS, prevRMS, imean)

        if citr < 5:
            print("Only " + str(citr) + " iterations in clipping at " + str(cfactor) + "-sigma")
            runWarningList.append("Only " + str(citr) + " iterations in clipping at " + str(cfactor) + "-sigma")

        l1 = imean - cfactor * iRMS
        l2 = imean + cfactor * iRMS

        PEAK = imean

        Osel = np.where((presid > l1) & (presid < l2))[0]

    elif (clipLEHM):
        print('\n -- Clipping at limits from LEHM')

        # Construct histogram of residual distribution using pre-defined bins 'hbins'
        histr = np.histogram(presid, hbins)
        amp = histr[0]
        pbins = hbins[1:] - 0.5 * hstep

        # Generate a smoothed distribution profile by averaging neighbouring bins
        winlen = 6  # 6ps smoothing
        smth = np.ones(winlen) / winlen
        hprofil = np.convolve(amp, smth, mode='same')

        # A histogram fit is performed on the front of the distribution using only the bins in front of the peak as defined by the index 'am'. To set 'am' the 20 highest bins are selected and the RMS of their residual bin values is calculated.  If this RMS is greater than 20ps, the highest indexes are removed until this is reached.  This is done to avoid double peaks in distributions. 'am' is taken as the mean of the remaining indexes.

        am = hprofil.argmax() + 2
        ami = hprofil.argsort()[-20:-1]

        #     print ( '       Peak is', "{:7.5f}".format(np.std(1.0*pbins[ami])), 'ps wide from LEHM to FEHM')
        while (np.size(ami) > 3) & (np.std(1.0 * pbins[ami]) > 5.0):
            #         print ( np.std(1.0*pbins[ami]) ,ami)
            d = ami.argmax()
            ami = np.delete(ami, d)

        am = int(np.mean(1.0 * ami)) + 1

        al = 0
        af = np.min(np.where(hprofil > 0.25 * np.max(hprofil)))

        al = am - int(50 / psecbin)
        if (PWadjust):
            al = am - int(3 * PWidth / psecbin)
        if (al > af):
            al = af
        if (al < 0):
            al = 0

        # p0 is the initial guess for the Gaussian fitting coefficients (A, mu and sigma)
        p0 = [np.max(hprofil), 0.0, cRMS]
        pp = hprofil.argmax()

        # Use Scipy function 'curve_fit' to fit a Gaussian function to the front of the distribution.  If the resulting Gaussian fit peak is not 90%-110% of that of the smoothed profile then adjust 'am' to include additional distribution bins further from the front and repeat fit.

        atmpt = 0
        while (atmpt < 12):
            atmpt = atmpt + 1
            try:
                if (al < 0):
                    al = 0
                coeff, var_matrix = curve_fit(gauss, hbins[al:am], amp[al:am], p0=p0)
            except  Exception as inst:
                print("Gaussian Fit Function - Except" + str(inst.args))
                coeff = np.zeros(3)

            if ((coeff[0] > 1.05 * hprofil[pp]) | (coeff[0] < 0.95 * hprofil[pp])):
                am = am + 2
                al = al - 1
                print("\t\tFront Gaussian Fit - Too Tall/Short - Attempt #" + str(atmpt))

            elif (PWadjust) & (coeff[2] > 2.5 * PWidth):
                am = am - 1
                al = al - 1
                print("\t\tFront Gaussian Fit - Too Wide - Attempt #" + str(atmpt))

            else:
                atmpt = 100

            if (al < 0):
                al = 0

        if ((coeff[0] > 1.05 * hprofil[pp]) | (coeff[0] < 0.95 * hprofil[pp])):
            runWarningList.append("Front Gaussian Fit - Too Tall/Short")

        if ((PWadjust) & (coeff[2] > 2.5 * PWidth)):
            print("Front Gaussian Fit RMS above limit for 10ps laser pulse")
            runWarningList.append("Front Gaussian Fit RMS above limit for 10ps laser pulse")

        if (coeff[0] == 0.0):
            print("Front Gaussian Fit Fail. " + str(neps) + " Observations")
            runWarningList.append("Front Gaussian Fit Fail. " + str(neps) + " Observations")
            l1 = LEHMlow
            l2 = LEHMupp
            Osel = np.where(aresid)[0]
        else:

            # Determine the Gaussian fit peak
            PEAK = coeff[1]
            PEAKi = abs(hbins - PEAK).argmin()
            PEAKm = coeff[0]

            # Determine the Gaussian fit leading edge half maximum
            gauss_hist_fit = gauss(hbins, *coeff)
            l = abs(gauss_hist_fit[0:PEAKi] - 0.5 * PEAKm).argsort()[0:6]
            IntpG = interpolate.interp1d(gauss_hist_fit[l], hbins[l] + 0.5 * hstep, kind='linear',
                                         fill_value="extrapolate")
            LEHM = float(IntpG(0.5 * PEAKm))

            # Determine the histogram profile falling edge half maximum
            l = abs(hprofil[am:-1] - 0.5 * PEAKm).argsort()[0:6] + am
            IntpG = interpolate.interp1d(hprofil[l], pbins[l], kind='linear', fill_value="extrapolate")
            FEHM = float(IntpG(0.5 * PEAKm))

            # Set residual clipping points and select data
            l1 = LEHM + LEHMlow
            l2 = LEHM + LEHMupp
            Osel = np.where((presid > l1) & (presid < l2))[0]


    else:
        Osel = np.where(aresid)[0]

    OUTresid = presid[Osel]
    OUTrng = Drng[Osel]
    OUTep = Depc[Osel]
    OUTdt = Ddatet[Osel]
    OUTmjd = Dmep[Osel]

    tRMS = np.std(OUTresid)

    ymin = PEAK - 400.0
    ymax = PEAK + 550.0
    ymin = PEAK - 3.0 * tRMS
    ymax = PEAK + 4.0 * tRMS
    if (ymax - ymin < 200):
        dif = 200 - (ymax - ymin)
        ymax = ymax + 0.5 * dif
        ymin = ymin - 0.5 * dif

    fbin = OUTep[0] - np.mod(OUTep[0], NPbin_length)
    fdtbin = OUTdt[0] - dt.timedelta(seconds=np.mod(OUTep[0], NPbin_length))
    lbin = OUTep[-1]
    if (lbin > fbin):
        NPbins = np.arange(fbin, lbin, NPbin_length)
    else:
        b1 = np.arange(fbin, 86400.0, NPbin_length)
        b2 = np.arange(0.0, lbin, NPbin_length)
        NPbins = np.concatenate((b1, b2))

    NPdtbins = np.array([fdtbin + dt.timedelta(seconds=i * NPbin_length) for i in range(len(NPbins))])

    Nav = list()
    Nstd = list()
    Nskw = list()
    Nkrt = list()
    Nep = list()
    Nmjd = list()
    Ndatet = list()
    NRng = list()
    Nnpts = list()
    Ndur = list()

    # calculate normal points from residuals
    for b in NPbins:
        sel = np.where((OUTep >= b) & (OUTep < b + NPbin_length))[0]
        if (np.size(sel) > minNPn):
            av = np.mean(OUTresid[sel])
            Nav.append(av)
            Nstd.append(2.0 * np.std(OUTresid[sel]))
            Nskw.append(skew(OUTresid[sel], axis=0, bias=True))
            Nkrt.append(kurtosis(OUTresid[sel], axis=0, fisher=True, bias=True))
            c = abs(OUTep[sel] - np.mean(OUTep[sel])).argmin()  # find closest OUTep epoch to NP mean epoch
            Nep.append(OUTep[sel[c]])
            Nmjd.append(OUTmjd[sel[c]])
            Ndatet.append(OUTdt[sel[c]])
            # create NP range values from mean epoch range and NP average residual
            NRng.append(OUTrng[sel[c]] * 1e-12 + 2.0 * (av - OUTresid[sel[c]]) * 1e-12)  # form NP range
            Nnpts.append(np.size(sel))
            maxNep = np.max(OUTep[sel])
            minNep = np.min(OUTep[sel])
            if (minNep > maxNep):
                maxNep = maxNep + 86400.0
            Ndur.append(maxNep - minNep)

    Nav = np.array(Nav)
    Nstd = np.array(Nstd)
    Nep = np.array(Nep)
    NRng = np.array(NRng)
    Nnpts = np.array(Nnpts)
    Ndur = np.array(Ndur)
    nN = np.size(Nep)

    # use laser fire rate to estimate return rate for NP. Instead of normal point length, use the interval between the latest and earliest epochs in the normal point bin.
    if frate != 0.0:
        rR = 100.0 * Nnpts / (Ndur * frate)
        # rR=100.0*Nnpts/(NPbin_length*frate)
    else:
        rR = np.zeros(np.size(Nnpts))

    if (NPout):
        # print ( normal point results and write to file)
        print('\n -- Normal Points')
        filenp = open("normalp.dat", "w")

        for i, epo in enumerate(Nep):
            print("\t11 " + "{:18.12f}".format(epo) + "     " + "{:14.12f}".format(
                NRng[i]) + "   KS 2 " + "{:6.1f}".format(NPbin_length) + ' ' + "{:5d}".format(
                Nnpts[i]) + ' ' + "{:9.1f}".format(Nstd[i]) + ' ' + "{:6.3f}".format(Nskw[i]) + ' ' + "{:6.3f}".format(
                Nkrt[i]) + "      0.0 " + "{:4.1f}".format(rR[i]) + " 0")
            filenp.write(
                "11 " + "{:18.12f}".format(epo) + "     " + "{:14.12f}".format(NRng[i]) + "   KS 2 " + "{:6.1f}".format(
                    NPbin_length) + ' ' + "{:5d}".format(Nnpts[i]) + ' ' + "{:9.1f}".format(
                    Nstd[i]) + ' ' + "{:6.3f}".format(Nskw[i]) + ' ' + "{:6.3f}".format(
                    Nkrt[i]) + "      0.0 " + "{:4.1f}".format(rR[i]) + " 0\n")

        filenp.close()

    # Plot the orbit ranges, range residuals from the orbit and residuals
    # from the adjusted orbit with the distribution histogram
    if (plotRES):

        print('\n -- Plot Results')
        fig = figure(figsize=(13, 8))
        figtext(0.05, 0.97, "Satellite Laser Range data from: " + dataf, fontsize=13, color="#113311")
        figtext(0.05, 0.942,
                'Station: ' + STAT_name + ' ' + STAT_id + '                              Satellite: ' + SATtarget_name.capitalize(),
                fontsize=12, color="#227722")
        figtext(0.975, 0.925, 'CPF: ' + CPFin.split('/')[-1], fontsize=9, verticalalignment='bottom',
                horizontalalignment='right')
        if (METap == False):
            figtext(0.975, 0.965, 'No Range Correction Applied From Local Meteorological Data', fontsize=10,
                    verticalalignment='bottom', horizontalalignment='right', color="#bb1111")
        figtext(0.9725, 0.63, 'Solve RMS: ' + "{:6.2f}".format(cRMS) + 'ps', fontsize=10, horizontalalignment='right',
                color="#227722")
        figtext(0.9725, 0.61, 'Final RMS: ' + "{:6.2f}".format(tRMS) + 'ps', fontsize=10, horizontalalignment='right',
                color="#227722")
        figtext(0.08, 0.6, 'Time Bias:   ' + "{:7.2f}".format(alnc * 8.64e7) + 'ms', fontsize=10,
                horizontalalignment='left', color="#227722")
        figtext(0.08, 0.58, 'Range Bias:  ' + "{:7.2f}".format(radc * 1e6) + 'm', fontsize=10,
                horizontalalignment='left', color="#227722")

        # set up grid for output plots
        gs1 = gridspec.GridSpec(1, 4)
        gs1.update(left=0.075, right=0.975, top=0.92, bottom=0.71, wspace=0.35)
        ax1 = subplot(gs1[0, 0:2])
        ax2 = subplot(gs1[0, 2:4])
        gs2 = gridspec.GridSpec(1, 4)
        gs2.update(left=0.075, right=0.975, top=0.65, bottom=0.075, wspace=0.075)
        ax3 = subplot(gs2[0, 0:3])
        ax4 = subplot(gs2[0, 3], sharey=ax3)
        ax1.tick_params(axis='both', which='major', labelsize=10)
        ax2.tick_params(axis='both', which='major', labelsize=10)
        ax3.tick_params('x', length=4, width=0.5, which='minor', color=[0.65, 0.65, 0.65], labelsize=7)
        ax3.tick_params('x', length=12, width=1.5, which='major', color=[0.5, 0.5, 0.5], labelsize=9)
        ax3.tick_params(axis='y', which='major', labelsize=10)
        ax4.tick_params(axis='x', which='major', labelsize=10)
        ax4.yaxis.set_visible(False)
        ax4.xaxis.set_visible(False)

        # set time axis limits
        xmin = np.min(Ddatet)
        xmax = np.max(Ddatet)

        # plot 2-way ranges vs time
        subplot(ax1)
        ax = gca()
        ax.ticklabel_format(useOffset=False)
        plot(Ddatet, Drng * 1.0e-12, '.', ms=1.0, color="#88aa88")
        if (np.size(ax1.get_xticks()) > 8):
            ax1.tick_params(axis='x', which='major', labelsize=9)
            xticks(rotation=10)
        ylabel("Range (s)", fontsize=10)
        xlabel("Epoch", fontsize=8)
        xlim(xmin, xmax)

        # plot residuals from CPF orbit
        subplot(ax2)
        ax = gca()
        ax.ticklabel_format(useOffset=False)

        plot(Ddatet, pltresy1, 'b.', ms=1.0, color="#338833", alpha=0.5)
        if (np.size(ax2.get_xticks()) > 8):
            ax2.tick_params(axis='x', which='major', labelsize=9)
            xticks(rotation=10)
        ylabel("CPF Residual (ns)", fontsize=10)
        xlabel("Epoch", fontsize=8)
        xlim(xmin, xmax)

        # plot final range residuals against time
        subplot(ax3, sharey=True)
        minor_ticks = np.array([fdtbin + dt.timedelta(seconds=i * 0.1 * NPbin_length) for i in range(10 * len(NPbins))])
        ax3.set_xticks(minor_ticks, minor=True)
        ax = gca()
        ax.ticklabel_format(useOffset=False)

        # control x axis labels at NP start values and limit the number plotted
        ax3.set_xticks(NPdtbins)
        if len(NPbins) > 15:
            for l, label in enumerate(ax3.get_xticklabels()):
                if np.mod(l, 2) != 0:
                    label.set_visible(False)
                else:
                    label.set_rotation(25)

        if NPbin_length > 60.0:
            ax1.xaxis.set_major_formatter(DateFormatter('%H:%M'))
            ax2.xaxis.set_major_formatter(DateFormatter('%H:%M'))
            ax3.xaxis.set_major_formatter(DateFormatter('%H:%M'))
        else:
            ax1.xaxis.set_major_formatter(DateFormatter('%H:%M:%S'))
            ax2.xaxis.set_major_formatter(DateFormatter('%H:%M:%S'))
            ax3.xaxis.set_major_formatter(DateFormatter('%H:%M:%S'))

        # plot range residuals and normal point results on chart
        mark = 1.5
        if (neps < 500):
            mark = 3.0
        plot(OUTdt, OUTresid, '.', ms=mark, color="#222222")  # ,alpha=0.35)
        ylim(ymin, ymax)
        ylabel("Fit Residual (ps)")
        xlabel("Epoch on date " + "{0}/{1}/{2}".format(Hd, Hmon, Hyr))
        xlim(xmin, xmax)
        errorbar(Ndatet, Nav, Nstd, fmt='.', ms=0.0, ecolor="#11bb11", elinewidth=2.0, capsize=4, capthick=2, zorder=3)
        plot(Ndatet, Nav, 's', ms=7.0, mew=2.0, mec="#11bb11", color="#b0ffa0", alpha=0.85, zorder=3)

        # plot horizontal histogram of range residuals
        subplot(ax4)
        histr = hist(OUTresid, hbins, orientation='horizontal', edgecolor=[0.0, 0.3, 0.0], color=[0.0, 0.5, 0.3],
                     lw=0.4)

        if (clipLEHM):
            # plot smoothed distribution profile. plot the fitted Gaussian profile. plot peak value. plot LEHM and FEHM
            plot(hprofil, pbins, 'k-', lw=1.1)
            if (coeff[0] != 0.0):
                plot(gauss_hist_fit, hbins + 0.5 * hstep, '-', lw=1.5, color="#ff5533")
                plot(coeff[0], coeff[1], 's', color="#ff5533", ms=6.0)
                plot(0.5 * PEAKm, LEHM, 's', color="#f0f000", ms=6.0)
                plot(0.5 * PEAKm, FEHM, 's', color="#ffffd0", ms=6.0)

        if (clipLEHM | clipsigma):
            # plot limit lines for user defined data clipping
            hmax = np.max(histr[0])
            plot([0.0, hmax], [l1, l1], '-', color="#555555", lw=1.0)
            plot([0.0, hmax], [l2, l2], '-', color="#555555", lw=1.0)

        # save image to file. Use ipass index if multi pass FR data file used
        pfol = 'pics/'
        if not os.path.exists(pfol):
            os.makedirs(pfol)
        if (savepassname != ''):
            if (ipass == -1):
                fig = pfol + savepassname + '.png'
            else:
                fig = pfol + savepassname + '_' + str(ipass) + '.png'
        elif (frdata != ''):
            if (ipass == -1):
                #fig + '_' + frdata.split('/')[-1][:-3] + 'png'
                fig = pfol + frdata.split('/')[-1][:-3] + '.png'# JRM mod
            else:
                fig = pfol + STAT_id + '_' + frdata.split('/')[-1][:-4] + '_' + str(ipass) + '.png'
        else:
            fig = pfol + STAT_id + '_' + SATtarget_name + '_crd_' + Ddatet[0].strftime("%Y%m%d_%H_00") + '.png'

        print("\tSave fig ", fig)
        savefig(fig, dpi=200)

    if (displayRES):
        show(False)

    # print ( warnings summary list to screen    )
    print("\n -- Summary: ")
    wList = np.concatenate((setupWarningList, runWarningList))
    if len(wList) == 0:
        print("\t Completed OK")
    else:
        for line in wList:
            print("\tWarning: ", line)
        if FRDloop:
            rawi = input(
                '\n ** Warnings in process, hit Enter to continue\n                            (q to quit)\n  ')
            if (rawi == 'q'):
                sys.exit()
    print("\n ")
