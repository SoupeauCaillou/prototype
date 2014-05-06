/*
    This file is part of Bzzz.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Bzzz is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Bzzz is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Bzzz.  If not, see <http://www.gnu.org/licenses/>.
*/

APP_PLATFORM := android-9
APP_PROJECT_PATH := $(call my-dir)/../
APP_BUILD_SCRIPT := $(call my-dir)/Android.mk
APP_MODULES = sac bzzz libpng libtremor libjsoncpp
APP_OPTIM := release
APP_ABI := armeabi-v7a armeabi
APP_STL := stlport_static

