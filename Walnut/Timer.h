//
//  Timer.h
//  Walnut
//
//  Created by Stephen H. Gerstacker on 2022-06-16.
//

#pragma once

#include <iostream>
#include <string>
#include <chrono>

namespace Walnut {

    class Timer {
      
    public:
        
        Timer() {
            Reset();
        }
        
        void Reset() {
            start = std::chrono::high_resolution_clock::now();
        }
        
        float Elapsed() {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count() * 0.001f * 0.001f * 0.001f;
        }
        
        float ElapsedMillis() {
            return Elapsed() * 1000.0f;
        }
        
    private:
        
        std::chrono::time_point<std::chrono::high_resolution_clock> start;
    };

    class ScopedTimer {
        
    public:
        
        ScopedTimer(const std::string& name)
            : name(name)
        {
        }
        
        ~ScopedTimer() {
            float time = timer.ElapsedMillis();
            std::cout << "[TIMER] " << name << " - " << time << "ms" << std::endl;
        }
        
    private:
        
        std::string name;
        Timer timer;
    };
}
