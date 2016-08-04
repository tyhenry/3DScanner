//
//  Commander.cpp
//  scannerControl
//
//  Created by Tyler on 8/3/16.
//
//

#include "Commander.hpp"

Commander::Commander(ofSerial* serialPtr){
    
    setSerial(serialPtr); // connection should already be established
    
}

