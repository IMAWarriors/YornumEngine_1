//




#include <iostream>
#include <string>
#include <vector>


template <typename T> int computeAverage (const std::vector<T> & nums) {

    float sum = 0;
    int count = 0;

    for (T val : nums) {
        sum += (float)(val);
        count++;
    }

    int result = (int)(sum/(float)count);

    return result;


}



int main () {

    
    int count;

    std::cin >> count;
    std::vector<float> numbers(count);

    for (int i = 0; i < count; i++) {
        std::cin >> numbers[i];
    }

    std::cout << "Average: " << computeAverage(numbers);

    return 0;
}