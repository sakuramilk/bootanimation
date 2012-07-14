/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "BootAnimation"

#include <cutils/properties.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <utils/Log.h>
#include <utils/threads.h>

#if defined(HAVE_PTHREADS)
# include <pthread.h>
# include <sys/resource.h>
#endif

#include "BootAnimation.h"

using namespace android;

// ---------------------------------------------------------------------------

int main(int argc, char** argv)
{
#if defined(HAVE_PTHREADS)
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_DISPLAY);
#endif

    char value[PROPERTY_VALUE_MAX];
    property_get("debug.sf.nobootanimation", value, "0");
	//property_get("persist.sys.nobootanimation", value, "0");
    int noBootAnimation = atoi(value);
    ALOGI_IF(noBootAnimation,  "boot animation disabled");

	property_get("persist.sys.nobootanimationwait", value, "0");
    int noBootAnimationWait = atoi(value);
    ALOGI_IF(noBootAnimationWait,  "boot animation wait disabled");

    char bootsoundFile[PROPERTY_VALUE_MAX];
    float bootsoundVolume = 0.2;
    property_get("persist.sys.nobootsound", value, "0");
    int noBootSound = atoi(value);
    ALOGI_IF(noBootSound,  "boot nobootsound disabled");
    if (!noBootSound) {
        property_get("persist.sys.boosound_file", bootsoundFile, "/system/media/bootsound.mp3");
        ALOGI("bootsound_file=%s", bootsoundFile);
        property_get("persist.sys.boosound_volume", value, "0.2");
        bootsoundVolume = atof(value);
        ALOGI("bootsound_volume=%f", bootsoundVolume);
    }

  if (!noBootAnimation) {

      //LOGI("[BOOT] setuid graphics");
        seteuid(1003);

        sp<ProcessState> proc(ProcessState::self());
        ProcessState::self()->startThreadPool();

        // create the boot animation object
        sp<BootAnimation> boot = new BootAnimation(
                                         noBootAnimationWait ? true : false,
                                         argc > 1 ? argv[1] : NULL,
                                         noBootSound ? NULL : (argc > 2 ? argv[2] : bootsoundFile),
                                         bootsoundVolume);

        IPCThreadState::self()->joinThreadPool();

        //LOGI("[BOOT] setuid root");
        seteuid(0);
        ALOGI("[BOOT] set sys.bootanim_completed");
        property_set("sys.bootanim_completed", "1");
    }
    return 0;
}
