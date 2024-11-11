#!/usr/bin/env python3
# -*- coding:utf-8 -*-


import time
# import sys
import ADS1263
#import RPi.GPIO as GPIO
import numpy as np
import MySQLdb
import argparse

# Local module with DB configuration
import sql_config

parser = argparse.ArgumentParser(
            description='Script to take acquisition with Mini-Nir hardware and store')
#parser.add_argument('-s', '--shot', type=int,
#                        help='ESTHER pulse Number ([1, ...])', default=180)
#parser.add_argument('-t', '--ploT',
#                        action='store_true', help='Plot Signals')
#parser.add_argument('-f','--filename', type=argparse.FileType('w'))

parser.add_argument('-d','--description', help='Experiment description')
parser.add_argument('-f','--filename', help='File to store data')
parser.add_argument('-l','--directory', default='dados', help='directory to store data')
parser.add_argument('-s','--series', default='H2O_', help='Series name')
parser.add_argument('-r', '--reference', type=int,
                        help='Reference Test number', default=1)

args = parser.parse_args()

if args.description:
    desc = args.description
else:
    desc = 'Sem descricao'

#filename = args.directory + '/' +args.series + timestr + '.csv'
if args.filename:
    filename = args.filename + '.csv'
else:
    timestr = time.strftime("%Y%m%d-%H%M%S")
    # filename = 'dados/agua' + timestr + '.csv'
    filename = args.series + timestr + '.csv'

filepath = args.directory + '/' + filename

print(filepath)
# exit()

REF = 5.22          # Modify according to actual voltage
                    # external AVDD and AVSS(Default), or internal 2.5V

NUM_SAMPLES = 6000
NUM_CHANNELS = 6

SQL = ('INSERT INTO tabelaEnsaios '
       '(Descricao, dateTime, filename, ensaioRef, '
       'LED1_avg, LED1_std, LED2_avg, LED2_std, LED3_avg, LED3_std, '
       'LED4_avg, LED4_std, LED5_avg, LED5_std, LED6_avg, LED6_std) '
       'VALUES (%s,current_timestamp(), %s, %s, '
       '%s, %s, %s, %s, %s, %s, '
       '%s, %s, %s, %s, %s, %s)')
try:
    mydb = MySQLdb.connect(
        host=sql_config.host,
        user=sql_config.user,
        passwd=sql_config.passwd,
        db=sql_config.db)

# prepare a cursor object
    
    cursor = mydb.cursor()
except MySQLdb._exceptions.OperationalError:
    print('Error connecting to SQL DB. Saving to file only')
#    exit()

try:
    ADC = ADS1263.ADS1263()
    if (ADC.ADS1263_init_ADC1('ADS1263_7200SPS') == -1):
        exit()
    ADC.ADS1263_SetMode(0)

    # rate test
    time_start = time.time()
    ADC_Value = np.zeros((NUM_SAMPLES, NUM_CHANNELS + 1))

    for i in range(NUM_SAMPLES):
        for c in range(NUM_CHANNELS):
        #ADC_Value.append(ADC.ADS1263_GetAll())
            raw = ADC.ADS1263_GetChannalValue(c)
            if(raw>>31 ==1):
                adcval = REF*2 - raw * REF / 0x80000000
                # print("ADC1 IN%d = -%lf" %(i, (REF*2 - ADC_Value[i] * REF / 0x80000000)))
            else:
                # adcval = REF*2 - raw * REF / 0x7fffffff
                # print("ADC1 IN%d = %lf" %(i, (ADC_Value[i] * REF / 0x7fffffff)))   # 32bit
                adcval = raw * REF / 0x7fffffff   # 32bit
            ADC_Value[i, c + 1] = adcval

        ADC_Value[i, 0] = time.time() - time_start

    time_end = time.time()
    # ADC.ADS1263_DAC_Test(1, 1)      # Open IN6
    # ADC.ADS1263_DAC_Test(0, 1)      # Open IN7
    
        
    ADC.ADS1263_Exit()

    print('frequency = ', NUM_SAMPLES / (time_end - time_start))
    print ('Acquisition Done. Saving Data to ', filepath)

# colocar aqui funcao para guardar ADC_Value em SQL
    a = np.average(ADC_Value, axis=0)
# Compute the standard deviation along the specified axis
    s = np.std(ADC_Value, axis=0)



#colocar aqui funcao para guardar ADC_Value em fich  .csv    
    np.savetxt(filepath, ADC_Value, fmt='%.5f', delimiter=',')

except IOError as e:
    print(e)
   
except KeyboardInterrupt:
    print("ctrl + c:")
    print("Program end")
    ADC.ADS1263_Exit()
    exit()


#sql = 'INSERT INTO tabelaEnsaios Ensaio, Descricao, dateTime, VALUES (NULL, )'
#    sql = """INSERT INTO tabelaEnsaios ( Descricao, dateTime, filename, ensaioRef, LED1_avg, LED1_std, LED2_avg, LED2_std, LED3_avg, LED3_std, LED4_avg, LED4_std, LED5_avg, LED5_std, LED6_avg, LED6_std) VALUES (%s, current_timestamp(), %s, %s, 
#%s, %s, %s, %s, %s, %s,
#%s, %s, %s, %s, %s, %s)"""
#    record = (desc,filename[6:],1,a[1],s[1],a[2],s[2],a[3],s[3],a[4],s[4],a[5],s[5],a[6],s[6])

record = (desc,  filename, 1,
          a[1], s[1], a[2], s[2], a[3], s[3],
          a[4], s[4], a[5], s[5], a[6], s[6])

# execute SQL query using execute() method.
try:
    # Executing the SQL command
    cursor.execute(SQL, record)
    # Commit your changes in the database
    mydb.commit()
    print ('Data Stored in DB')
    mydb.close()
except:
    # Rolling back in case of error
    print('Data not stored....')
    mydb.rollback()

# disconnect from server
