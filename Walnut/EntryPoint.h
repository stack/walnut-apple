//
//  EntryPoint.h
//  Walnut
//
//  Created by Stephen H. Gerstacker on 2022-06-14.
//

#pragma once

extern Walnut::Application* Walnut::CreateApplication(int argc, char** argv);

bool IsApplicationRunning = true;

namespace Walnut {
    int Main(int argc, char** argv) {
        while (IsApplicationRunning) {
            Walnut::Application *application = Walnut::CreateApplication(argc, argv);
            application->Run();
            delete application;
        }

        return 0;
    }
}

int main(int argc, char** argv) {
    return Walnut::Main(argc, argv);
}
