//
//  all_pzb.h
//  pzb
//
//  Created by tihmstar on 02.10.18.
//  Copyright © 2018 tihmstar. All rights reserved.
//

#ifndef all_pzb_h
#define all_pzb_h

#ifdef DEBUG //this is for developing with Xcode
#define PZB_VERSION_COMMIT_COUNT "Debug"
#define PZB_VERSION_COMMIT_SHA "Build: " __DATE__ " " __TIME__
#else
#include <config.h>
#endif

#endif /* all_pzb_h */
