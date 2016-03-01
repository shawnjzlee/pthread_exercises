//  =============== BEGIN ASSESSMENT HEADER ================
/// @file assign2/main.cpp
/// @brief Assignment2 for Cs 12v Winter 2014
///
/// @author Stavan Thaker [sthak004@ucr.edu]
/// @date January 21, 2014
///
/// @par Enrollment Notes
///     Lecture Section: <e.g. 001>
/// @par
///     Lab Section: <e.g. 021>
/// @par
///     TA: <TA name>
///
/// @par Plagiarism Section
/// I hereby certify that the code in this file
/// is ENTIRELY my own original work.
//  ================== END ASSESSMENT HEADER ===============


#include <iostream>
#include <fstream>
#include <cstdlib>
#include <iomanip>

#include <chrono>

using namespace std;


void initMatrix(double plate[6][8], double top, double right, double bottom, double left){
    for(int topPart = 0; topPart < 8; topPart++){
        plate[0][topPart] = top;
    }
    for(int rightPart = 1; rightPart < 5; rightPart++){
        plate[rightPart][7] = right;
    }
    for(int bottomPart = 5; bottomPart < 8; bottomPart++){ //bottom part might be a 0
        plate[5][bottomPart] = bottom;
    }
    for(int leftPart = 1; leftPart < 5; leftPart++){
        plate[leftPart][0] = left;
    }
}

double updateGrid(double grid[6][8]){
    double initial = 0;
    double different = 0;
    double maxDifference = 0;
    for(int i = 1; i < 5; i++){          //  i = inner row
        for(int j = 1; j < 7; j++){      //  j = inner column
            initial = grid[i][j];
            grid[i][j] = ((grid[i-1][j] + grid[i][j-1] + grid[i][j+1] +
                           grid[i+1][j])/4);
            different = grid[i][j] - initial;
            if(different > maxDifference){
                maxDifference = different;
            }
        }
    }
    return maxDifference;
}

int main()
{
    //start timer here
    //using namespace std::chrono;
    //high_resolution_clock::time_point t1 = high_resolution_clock::now();
    
    
    ifstream inFS;
    ofstream outFS;
    double top = 0.0;
    double right = 0.0;
    double bottom = 0.0;
    double left = 0.0;
    double tolerance = 0.0;
    double matrix[6][8] = {0};
    
    string inputFile = "";
    string  outputFile = "";
    cout << "Enter the name of the input file: ";
    cin >> inputFile;
    
    cout << "Enter the name of the output file: ";
    cin >> outputFile;
    
    
    
    inFS.open(inputFile.c_str());
    if (!inFS.is_open()) {
        cout << "Could not open file " << inputFile << endl;
        return 1;
    }
    
    outFS.open(outputFile.c_str());
    if(!outFS.is_open()){
        cout << "Could not open file " << outputFile << endl;
        return 1;
    }
    
    
    inFS >> top;
    inFS >> right;
    inFS >> bottom;
    inFS >> left;
    inFS >> tolerance;

    
    /*double test[5];
    double x;
    int a = 0;
    while(inFS >> x){
        if(a == 0){
            test[0] = x;
            a++;
        }
        else if(a == 1){
            test[1] = x;
            a++;
        }
        else if(a == 2){
            test[2] = x;
            a++;
        }
        else if(a == 3){
            test[3] = x;
            a++;
        }
        else if(a == 4){
            test[4] = x;
            a++;
        }
    }
    top = test[0];
    right = test[1];
    bottom = test[2];
    left = test[3];
    tolerance = test[4];*/
    
    
    initMatrix(matrix, top, right, bottom, left);
    
    
    double variable;
    do{
        variable = updateGrid(matrix);
        
        
    }while(variable > tolerance);
    
    
    //end timer here
    //high_resolution_clock::time_point t2 = high_resolution_clock::now();
    
    //compute difference between t1 and t2
    //duration<double> time_span = duration_cast<seconds>(t2 - t1);
    
    for(int i = 1; i < 5; i++){
        for(int j =1; j <7; j++){
            outFS << matrix[i][j] << " ";
        }
        outFS << endl;
    }
    
    /* Outputs the run time of the program */
    //outFS << time_span << endl;

    return 0;
}

