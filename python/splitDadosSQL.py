#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Apr 1 12:11:30 2021
@author: Bernardo Carvalho
"""

# import sys
import numpy as np
import MySQLdb
import argparse

# Local module with DB configuration
import sql_config

parser = argparse.ArgumentParser(
            description='Script to take acquisition with Mini-Nir hardware and store')

parser.add_argument('-e', '--ensaio', type=int,
                        help='Número de ensaio Test', default=20)
parser.add_argument('-l','--directory', default='dados', help='directory to get data file')
#parser.add_argument('-u', '--ultimo',
#                        action='store_true', help='Get Last ensaio')

args = parser.parse_args()
#if len(sys.argv) > 1:
#    Ensaio= sys.argv[1]
#else:

print(f"Ensaio {args.ensaio}")
SQL = "SELECT Ensaio FROM tabelaEnsaios ORDER by Ensaio DESC LIMIT 1"
try:
    mydb = MySQLdb.connect(
        host=sql_config.host,
        user=sql_config.user,
        passwd=sql_config.passwd,
        db=sql_config.db)

# prepare a cursor object
    
    cursor = mydb.cursor()
    cursor.execute(SQL)
    last = cursor.fetchone()
    print(f"Ultimo Ensaio {last}")

except MySQLdb._exceptions.OperationalError:
    print('Error connecting to SQL DB.')
    exit()


sql = f'SELECT filename from tabelaEnsaios where Ensaio = {args.ensaio}'
#sql1= sql %Ensaio
print(sql)
#record = (Ensaio)
cursor.execute(sql)
result = cursor.fetchone()
if result is None:
    print("Erro: ficheiro nao existe na DB")
    exit()

# elif result:
#filepath = args.directory + '/' + filename
filepath = args.directory + '/' + result[0]
# filename = "dados/" + result[0]

try:
    d = np.genfromtxt(filepath, delimiter=',')
except FileNotFoundError:
    print(f"Ficheiro {filepath} nao encontrado!")
    exit()

print(f"Ficheiro \'{filepath} lido!")

NSAMP = 600
line = 0
SQL = ("INSERT INTO dadosPLS (EnsaioId, line, initTime, "
        "LED1_avg, LED1_std, LED2_avg, LED2_std, "
        "LED3_avg, LED3_std, LED4_avg, LED4_std, "
        "LED5_avg, LED5_std, LED6_avg, LED6_std) "
        "VALUES (%s, %s, %f, "
        "%s, %s, %s, %s, %s, %s, "
        "%s, %s, %s, %s, %s, %s)" )

line = 0
# exit()
for i in range(0, d.shape[0], NSAMP):
    a = np.average(d[i:i+NSAMP, :], axis=0)
    s = np.std(d[i:i+NSAMP, :], axis=0)
    record = (args.ensaio, line, d[i, 0], a[1], s[1], a[2], s[2],
            a[3], s[3], a[4], s[4], a[5], s[5], a[6], s[6])
    sqlInsert = SQL%record
    print(sqlInsert)
    try:
        # Executing the SQL command
        cursor.execute(sqlInsert)
        #cursor.execute(sql,record)

        mydb.commit()
    except:
        # Rolling back in case of error
        print('Data not saved.')
        mydb.rollback()
    line = line + 1 

# disconnect from server

mydb.close()
