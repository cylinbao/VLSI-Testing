#include <iostream>
#include <ctime>
#include <bitset>
#include <string>
#include <fstream>

using namespace std;
const unsigned PatternNum = 16;

void evaluate();
void printIO(unsigned idx);

bitset<PatternNum> G_G1[2];
bitset<PatternNum> G_G2[2];
bitset<PatternNum> G_G3[2];
bitset<PatternNum> G_G4[2];
bitset<PatternNum> G_G5[2];
bitset<PatternNum> G_PO_G16[2];
bitset<PatternNum> G_G16[2];
bitset<PatternNum> G_PO_G17[2];
bitset<PatternNum> G_G17[2];
bitset<PatternNum> G_n60[2];
bitset<PatternNum> G_net14[2];
bitset<PatternNum> G_net17[2];
bitset<PatternNum> G_net25[2];
bitset<PatternNum> G_net18[2];
bitset<PatternNum> temp;
ofstream fout("c17.out",ios::out);

int main()
{
clock_t time_init, time_end;
time_init = clock();
G_G1[0] = 16191 ;
G_G1[1] = 15914 ;
G_G2[0] = 63350 ;
G_G2[1] = 55122 ;
G_G3[0] = 34769 ;
G_G3[1] = 1984 ;
G_G4[0] = 10055 ;
G_G4[1] = 9280 ;
G_G5[0] = 57273 ;
G_G5[1] = 49584 ;

evaluate();
printIO(16);

G_G1[0] = 16378 ;
G_G1[1] = 15930 ;
G_G2[0] = 63482 ;
G_G2[1] = 55104 ;
G_G3[0] = 34790 ;
G_G3[1] = 1894 ;
G_G4[0] = 10015 ;
G_G4[1] = 9245 ;
G_G5[0] = 57304 ;
G_G5[1] = 49544 ;

evaluate();
printIO(8);

time_end = clock();
cout << "Total CPU Time = " << double(time_end - time_init)/CLOCKS_PER_SEC << endl;
system("ps aux | grep a.out ");
return 0;
}
void evaluate()
{
G_net17[0] = G_G1[0] ;
G_net17[1] = G_G1[1] ;
G_net17[0] &= G_G3[0] ;
G_net17[1] &= G_G3[1] ;
temp = G_net17[0] ;
G_net17[0] = ~G_net17[1] ;
G_net17[1] = ~temp ;
G_n60[0] = G_G2[0] ;
G_n60[1] = G_G2[1] ;
G_n60[0] |= G_G5[0] ;
G_n60[1] |= G_G5[1] ;
temp = G_n60[0] ;
G_n60[0] = ~G_n60[1] ;
G_n60[1] = ~temp ;
G_net14[0] = G_G3[0] ;
G_net14[1] = G_G3[1] ;
G_net14[0] &= G_G4[0] ;
G_net14[1] &= G_G4[1] ;
temp = G_net14[0] ;
G_net14[0] = ~G_net14[1] ;
G_net14[1] = ~temp ;
G_net18[0] = G_net14[0] ;
G_net18[1] = G_net14[1] ;
G_net18[0] &= G_G2[0] ;
G_net18[1] &= G_G2[1] ;
temp = G_net18[0] ;
G_net18[0] = ~G_net18[1] ;
G_net18[1] = ~temp ;
G_net25[0] = G_net14[0] ;
G_net25[1] = G_net14[1] ;
temp = G_net25[0] ;
G_net25[0] = ~G_net25[1] ;
G_net25[1] = ~temp ;
G_G16[0] = G_net17[0] ;
G_G16[1] = G_net17[1] ;
G_G16[0] &= G_net18[0] ;
G_G16[1] &= G_net18[1] ;
temp = G_G16[0] ;
G_G16[0] = ~G_G16[1] ;
G_G16[1] = ~temp ;
G_G17[0] = G_n60[0] ;
G_G17[1] = G_n60[1] ;
G_G17[0] |= G_net25[0] ;
G_G17[1] |= G_net25[1] ;
temp = G_G17[0] ;
G_G17[0] = ~G_G17[1] ;
G_G17[1] = ~temp ;
G_PO_G16[0] = G_G16[0] ;
G_PO_G16[1] = G_G16[1] ;
G_PO_G17[0] = G_G17[0] ;
G_PO_G17[1] = G_G17[1] ;
}
void printIO(unsigned idx)
{
for (unsigned j=0; j<idx; j++)
{        if(G_G1[0][j]==0)
        {
            if(G_G1[1][j]==1)
                fout<<"F";
            else
                fout<<"0";
        }
        else
        {
            if(G_G1[1][j]==1)
                fout<<"1";
            else
                fout<<"2";
        }
        if(G_G2[0][j]==0)
        {
            if(G_G2[1][j]==1)
                fout<<"F";
            else
                fout<<"0";
        }
        else
        {
            if(G_G2[1][j]==1)
                fout<<"1";
            else
                fout<<"2";
        }
        if(G_G3[0][j]==0)
        {
            if(G_G3[1][j]==1)
                fout<<"F";
            else
                fout<<"0";
        }
        else
        {
            if(G_G3[1][j]==1)
                fout<<"1";
            else
                fout<<"2";
        }
        if(G_G4[0][j]==0)
        {
            if(G_G4[1][j]==1)
                fout<<"F";
            else
                fout<<"0";
        }
        else
        {
            if(G_G4[1][j]==1)
                fout<<"1";
            else
                fout<<"2";
        }
        if(G_G5[0][j]==0)
        {
            if(G_G5[1][j]==1)
                fout<<"F";
            else
                fout<<"0";
        }
        else
        {
            if(G_G5[1][j]==1)
                fout<<"1";
            else
                fout<<"2";
        }
fout<<" ";
        if(G_PO_G16[0][j]==0)
        {
            if(G_PO_G16[1][j]==1)
                fout<<"F";
            else
                fout<<"0";
        }
        else
        {
            if(G_PO_G16[1][j]==1)
                fout<<"1";
            else
                fout<<"2";
        }
        if(G_PO_G17[0][j]==0)
        {
            if(G_PO_G17[1][j]==1)
                fout<<"F";
            else
                fout<<"0";
        }
        else
        {
            if(G_PO_G17[1][j]==1)
                fout<<"1";
            else
                fout<<"2";
        }
fout<<endl;
}
}
