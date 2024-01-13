/*
 * atca-mimo32-v2-test.cpp
 *
 * Copyright (c) 2024-present,  IPFN-IST
 * All rights reserved.
 *
 * This source code is licensed under BSD-style license (found in the
 * LICENSE file in the root directory of this source tree)
 */

#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <getopt.h>
#include <cstring> /* strerror*/
//#include <string>
//#include <sstream>
//#include <stdio.h>
//#include <stdlib.h>
//#include <fstream>
//#include "math.h"
//#include "stdint.h"
//#include <string.h>

#include "libsarspec.h"

//https://stackoverflow.com/questions/6749621/how-to-create-a-high-resolution-timer-in-linux-to-measure-program-performance
uint64_t timespecDiff(timespec start, timespec end)
{
    return ((end.tv_sec * 1000000000UL) + end.tv_nsec) -
        ((start.tv_sec * 1000000000UL) + start.tv_nsec);
}

namespace sarspec_test
{
    using namespace std;

    int numberOfAcquisitionRounds = 0;
    int deviceNumber = 0;
    int integrationTime = 3;
    bool extTrigger = false;
    string outputFileName = "data";
    bool binaryFile = false;
    bool testLib = false;

    void writeDataFile(FILE* oFile, int nPixels, double* xdata,
            double* ydata)
    {
        for (int i = 0; i < nPixels; ++i) {
            fprintf(oFile, "%5.2f; %8.2f\n", xdata[i], ydata[i]);
        }
    }

    int processInputs(int argc, char* argv[])
    {
        int c;
        while ((c = getopt(argc, argv, "bf:i:lth")) != -1) {
            switch (c) {
                case 'b':
                    binaryFile = true;
                    break;
                case 'f':
                    outputFileName = optarg;
                    break;
                case 'i':
                    integrationTime = atoi(optarg);
                    break;
                case 'l':
                    testLib = true;
                    break;
                case 't':
                    extTrigger = true;
                    break;
                case 'h':
                    printf("All parameters are optional. If parameter is not specified then the default values will be used.\n \
                            -b - write Binary file\n \
                            -f [STRING] - output File name\n \
                            -i [INT] - Integration time (ms) \n \
                            -l - test Lib\n \
                            -t - use external Trigger\n \
                            -h - print this help\n");
                    /*
                       -a - acquire Data at 2MSPS \
                    // -a [INT] - number of acquisition rounds\n 
                    -b [INT] - buffer size in MB\n \
                    -d [INT] - device number \n \
                    -g [INT] - hold samples. (mas 63) \n \
                    -s - use software trigger\n \
                    -p [UINT] - chopper period, set to 0 to disable\n \
                    -o [STRING] - output file name\n \
                    -n [INT] - number of samples\n \
                    -t [LONG] - timeout miliseconds\n \
                    -e [INT[32]] - interlock electrical offsets\n \
                    -w [FLOAT[8]] - interlock wiring offsets\n \
                    -k [FLOAT[10]] - interlock parameters\n \
                    -y - write binary file\n \
                    -z [INT] - DMA buffer size in kB\n \
                    */
                    printf("e.g. %s -t\n", argv[0]);

                    exit(EXIT_SUCCESS);
                default:
                    fprintf(stderr, "Unknown command '%s'.\n",  argv[0]);
                    abort();
            }
        }
        return EXIT_SUCCESS;
    }

    int runTest(int argc, char* argv[])
    {
        int *connect;
        bool temp = false;
        int nPixel = 0;
        double *xdata, *ydata;
        string outputFileN;
        FILE *outputFile;
        timespec  startTime, endTime;

        processInputs(argc, argv);

        connect = Connect();
        if(connect[0] == 0)
        {
            cout << "Connection failed!!!" << endl;
            exit(EXIT_FAILURE);
        }
        else
        {
            if(connect[1]==0)  //STD
            {
                nPixel = 2048;
                cout << "spec std connected, nPixel:" << nPixel << endl;
            }
            if(connect[1]==1)  //RES+
            {
                //cout << "spec res+ connected!!!" << endl;
                nPixel = 3648;
                cout << "spec std connected, nPixel:" << nPixel << endl;
            }
        }
        if(testLib)
        {
            LibTest();
            Disconnect();
            return EXIT_SUCCESS;
        }
        //else 
        //Get data
        //double *ydata = YData(true,0);
        bool ledOn = false;
        for(int i = 0; i < 3; i++){
            usleep(100000);
            bool temp = LED(ledOn);
            ledOn = not ledOn;
        }

        temp = ChangeIntegrationTime(integrationTime);

        if(temp)
            cout << "Integration Time Changed to " << integrationTime << " ms." << endl;
        else
        {
            cout << "ChangeIntegrationTime command Failed!" << endl;
            return EXIT_FAILURE;
        }

        //𝑊𝑎𝑣𝑒𝑙𝑒𝑛𝑔𝑡h𝑃𝑖𝑥𝑒𝑙𝑁 = 𝐶0 + 𝐶1 𝑃𝑖𝑥𝑒𝑙𝑁 + 𝐶2 𝑃𝑖𝑥𝑒𝑙𝑁2 + 𝐶3 𝑃𝑖𝑥𝑒𝑙𝑁3
        //
        clock_gettime(CLOCK_MONOTONIC, &startTime);
        xdata = XData(0, 1, 0, 0);
        /*
         * double* YData(bool TriggerIN, int TriggerInDelay )
         * *
         * @brief Get intensity array.
         * @param TriggerIN:
         True – Acquires intensity array by “trigger in” signal
         False – Acquires intensity array by internal trigger
         * @param TriggerInDelay:
         *       Interval between 0 and MAXINT.
         * @return EXIT_SUCCESS (0) if successful, EXIT_FAILURE (1) otherwise
         */
        ydata = YData(extTrigger, 0);
        clock_gettime(CLOCK_MONOTONIC, &endTime);
        //End timer
        uint64_t timeDiff = timespecDiff(startTime, endTime);
        cout << endl << "Read Time: "<< timeDiff/1000 << " ms" << endl;	

        //bool temp = Disconnect();

        if(Disconnect())
        {
            cout << "Device Disconnected!" << endl;	
        }
        else
        {
            cout << "Disconnected Failed!" << endl;	
            return EXIT_FAILURE;
        }
        if (binaryFile) {
            outputFileN = outputFileName + ".bin";
            outputFile = fopen(outputFileN.c_str(), "wb");
            if (outputFile != NULL) {
                fwrite(xdata, sizeof(double), nPixel, outputFile);
                fwrite(ydata, sizeof(double), nPixel, outputFile);
                fclose(outputFile);
            }
            else {
                cerr << "Failed to open " <<  outputFileN << ".bin output file: " << strerror(errno) << endl;
                return EXIT_FAILURE;
            }
        }
        else {

            outputFileN = outputFileName + ".dat";
            outputFile = fopen(outputFileN.c_str(), "w");
            if (outputFile != NULL) {
                writeDataFile(outputFile, nPixel, xdata, ydata);
                fclose(outputFile);
            }
            else {
                cerr << "Failed to open " <<  outputFileN << 
                    " output file: " << strerror(errno) << endl;
            }
        }

        return EXIT_SUCCESS;
    }
}   //namespace sarspec_test

int main(int argc, char* argv[])
{
    return sarspec_test::runTest(argc, argv);

}

//  vim: syntax=cpp ts=4 sw=4 sts=4 sr et 
