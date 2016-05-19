#include <iostream>
#include <iomanip>

using namespace std;


//i belive this is the polynomial input
double func(double x){return x*x;}


int main()
{
    cout << "Please enter left bound ";
    double left;
    cin >> left;
    
    cout << "Please enter right bound ";
    double right;
    cin >> right;
    
    cout << "Please enter a # of partitions (>0): "; //how many rectangles
    
    double par;
    cin >> par;
    
    double width = ((right - left) / par);
    double total = 0;
    
    for(int i = 0; i < par; i++){
        total += func(left) * width;
        left += width;
    }
    
    cout << "The integral is: " << total << endl;
    
    return 0;
    
}