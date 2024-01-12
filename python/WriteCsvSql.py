#!/usr/bin/pythona3
# -*- coding:utf-8 -*-
"""
A program to read data from the High-Precision-AD-HAT
and save to a CSV file
"""

import time
import sys
import numpy as np
import MySQLdb

from MiniNirAdc import MiniNirAdc

NUM_SAMPLES = 6000
NUM_CHANNELS = 6
SQL_INSERT = """INSERT INTO tabelaEnsaios ( Descricao, dateTime, filename, ensaioRef, 
LED1_avg, LED1_std, LED2_avg, LED2_std, LED3_avg, LED3_std, LED4_avg, LED4_std, 
LED5_avg, LED5_std, LED6_avg, LED6_std) VALUES (%s, current_timestamp(), %s, %s,
%s, %s, %s, %s, %s, %s,
%s, %s, %s, %s, %s, %s)"""

timestr = time.strftime("%Y%m%d-%H%M%S")


if len(sys.argv) > 1:
    desc = str(sys.argv[1])
else:
    desc = 'NoDescription'

if len(sys.argv) > 2:
    filename = str(sys.argv[1])
else:
    filename = 'dados/BI_' + timestr + '.csv'

try:
    adc = MiniNirAdc()
    if (adc.open() == -1):
        print('Open Error')
        exit()

    adcVals = adc.read(num_samples=60, num_channels=6)
    adc.close()
    
    print('Acquisition Done. Saving Data to ' + filename)
    np.savetxt(filename, adcVals, fmt='%.5f', delimiter=',')

    # Compute the average deviation along the specified axis
    avg = np.average(adcVals, axis=0)
    # Compute the standard deviation along the specified axis
    stdev = np.std(adcVals, axis=0)

    # mydb = MySQLdb.connect(host="epics.ipfn.tecnico.ulisboa.pt",
    mydb = MySQLdb.connect(read_default_file="./my.cnf",
            db='miniNirDB')
    # prepare a cursor object
    cursor = mydb.cursor()

    #record = (desc,filename[6:],1,a[1],s[1],a[2],s[2],a[3],s[3],a[4],s[4],a[5],s[5],a[6],s[6])
    record = (desc, filename[6:], 1,)
    # append avg and stdev values
    for i in range(NUM_CHANNELS):
        record = record + (avg[i+1],stdev[i+1])
    try:
        # Executing the SQL command
        cursor.execute(SQL_INSERT, record)
        # Commit your changes in the database
        mydb.commit()
        print('Data Stored in DB')
    except:
        # Rolling back in case of error
        print('Data not stored....')
        print(record)
        mydb.rollback()

    # disconnect from server
    mydb.close()

except IOError as e:
    print(e)

except KeyboardInterrupt:
    print("ctrl + c:")
    print("Program end")
    exit()

# vim: sta:et:sw=4:ts=4:sts=4
