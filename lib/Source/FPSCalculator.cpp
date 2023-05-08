/* Allied Vision V4L2Viewer - Graphical Video4Linux Viewer Example
   Copyright (C) 2022 Allied Vision Technologies GmbH

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.  */

#include "FPSCalculator.h"
#include <QMutexLocker>

void FPSCalculator::trigger() {
    QMutexLocker lock(&mutex);
    auto const timestamp = std::chrono::high_resolution_clock::now().time_since_epoch();
    timestamps.push(std::chrono::duration_cast<std::chrono::microseconds>(timestamp).count());

    if(timestamps.size() >= 2) {
        fps = 1000000.0 * double(timestamps.size() - 1) / double(timestamps.back() - timestamps.front());

        if(timestamps.size() == 6) {
            timestamps.pop();
        }
    }
}

double FPSCalculator::getFPS() {
    QMutexLocker lock(&mutex);
    return fps;
}

void FPSCalculator::clear() {
    QMutexLocker lock(&mutex);
    fps = 0.0;
    timestamps = std::queue<uint64_t>();
}
