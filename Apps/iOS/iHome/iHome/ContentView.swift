//
//  ContentView.swift
//  iHome
//
//  Created by Julien Geneste on 09/05/2020.
//  Copyright © 2020 Kariboo. All rights reserved.
//

import LocalAuthentication
import SwiftUI

struct ContentView: View {
    
    @State private var isUnlocked = false
    
    var body: some View {
        Text("Hello, World!")
    }
    
    func authenticate() {
        let context = LAContext()
        var error: NSError?
        
        if context.canEvaluatePolicy(.deviceOwnerAuthenticationWithBiometrics, error: &error) {
            let reason = "We need to unlock your data."
        
            context.canEvaluatePolicy(.deviceOwnerAuthenticationWithBiometrics, localizedReason: reason) {
                success, authenticationError in DispatchQueue.main.async {
                    if success {
                        self.isUnlocked = true
                    } else {
                        
                    }
                }
            }
        }
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
