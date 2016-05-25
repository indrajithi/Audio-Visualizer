#include <iostream>
#include <vector>
 
void print_vec(const std::vector<int>& vec)
{
    for (auto x: vec) {
         std::cout << ' ' << x;
    }
    std::cout << '\n';
}
 
int main ()
{
    std::vector<int> array(0);
   
     
    std::vector<int> vec(3,100);

    
    print_vec(vec);
 


    auto it = vec.begin();

    auto at = array.end();
    
    for(int i=0; i< 10; i++){
        at = array.insert(at,i);
        at = array.end();

    }
    array.insert(at,2,20);
  //  print_vec(array);
    at = array.end();

    at = array.insert(at,100);
    at = array.insert(at,200);
    print_vec(array);
    
    it = vec.insert(it, 200);
    print_vec(vec);
    //print_vec(array);
    vec.insert(it,2,300);
    print_vec(vec);
 
    // "it" no longer valid, get a new one:
    print_vec(vec);
    it = vec.begin();
 
    std::vector<int> vec2(2,400);
    vec.insert(it+2, vec2.begin(), vec2.end());
    print_vec(vec);
 
    int arr[] = { 501,502,503 };
    vec.insert(vec.begin(), arr, arr+3);
    print_vec(vec);
}