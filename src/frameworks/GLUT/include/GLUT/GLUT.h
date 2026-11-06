/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef _GLUT_H_
#define _GLUT_H_

#import <Foundation/Foundation.h>

#import <GLUT/NSApplicationDelegate.h>
#import <GLUT/NSWindowDelegate.h>
#import <GLUT/GLUTWindow.h>
#import <GLUT/GLUTView.h>
#import <GLUT/GLUTPreferencesController.h>
#import <GLUT/GLUTMenuItem.h>
#import <GLUT/GLUTMenu.h>
#import <GLUT/GLUTUnknownView.h>
#import <GLUT/GLUTClipboardController.h>
#import <GLUT/GLUTApplication.h>

void* __glutGetFCB(void);
void* __glutSetFCB(void);
void* gleCreateGC(void);
void* gleExtrusion(void);
void* gleGetJoinStyle(void);
void* gleGetNumSlices(void);
void* gleHelicoid(void);
void* gleLathe(void);
void* glePolyCone(void);
void* glePolyCylinder(void);
void* gleScrew(void);
void* gleSetJoinStyle(void);
void* gleSetNumSlices(void);
void* gleSpiral(void);
void* gleSuperExtrusion(void);
void* gleTextureMode(void);
void* gleToroid(void);
void* gleTwistExtrusion(void);
void* glutAddMenuEntry(void);
void* glutAddSubMenu(void);
void* glutAttachMenu(void);
void* glutBitmapCharacter(void);
void* glutBitmapLength(void);
void* glutBitmapWidth(void);
void* glutButtonBoxFunc(void);
void* glutChangeToMenuEntry(void);
void* glutChangeToSubMenu(void);
void* glutCheckLoop(void);
void* glutCopyColormap(void);
void* glutCreateMenu(void);
void* glutCreateSubWindow(void);
void* glutCreateWindow(void);
void* glutDestroyMenu(void);
void* glutDestroyWindow(void);
void* glutDetachMenu(void);
void* glutDeviceGet(void);
void* glutDialsFunc(void);
void* glutDisplayFunc(void);
void* glutEnterGameMode(void);
void* glutEntryFunc(void);
void* glutEstablishOverlay(void);
void* glutExtensionSupported(void);
void* glutForceJoystickFunc(void);
void* glutFullScreen(void);
void* glutGameModeGet(void);
void* glutGameModeString(void);
void* glutGet(void);
void* glutGetColor(void);
void* glutGetMenu(void);
void* glutGetModifiers(void);
void* glutGetProcAddress(void);
void* glutGetWindow(void);
void* glutHideOverlay(void);
void* glutHideWindow(void);
void* glutIconifyWindow(void);
void* glutIdleFunc(void);
void* glutIgnoreKeyRepeat(void);
void* glutInit(void);
void* glutInitDisplayMode(void);
void* glutInitDisplayString(void);
void* glutInitWindowPosition(void);
void* glutInitWindowSize(void);
void* glutJoystickFunc(void);
void* glutKeyboardFunc(void);
void* glutKeyboardUpFunc(void);
void* glutLayerGet(void);
void* glutLeaveGameMode(void);
void* glutMainLoop(void);
void* glutMenuStateFunc(void);
void* glutMenuStatusFunc(void);
void* glutMotionFunc(void);
void* glutMouseFunc(void);
void* glutOverlayDisplayFunc(void);
void* glutPassiveMotionFunc(void);
void* glutPopWindow(void);
void* glutPositionWindow(void);
void* glutPostOverlayRedisplay(void);
void* glutPostRedisplay(void);
void* glutPostWindowOverlayRedisplay(void);
void* glutPostWindowRedisplay(void);
void* glutPushWindow(void);
void* glutRemoveMenuItem(void);
void* glutRemoveOverlay(void);
void* glutReportErrors(void);
void* glutReshapeFunc(void);
void* glutReshapeWindow(void);
void* glutSetColor(void);
void* glutSetCursor(void);
void* glutSetIconTitle(void);
void* glutSetKeyRepeat(void);
void* glutSetMenu(void);
void* glutSetWindow(void);
void* glutSetWindowTitle(void);
void* glutSetupVideoResizing(void);
void* glutShowOverlay(void);
void* glutShowWindow(void);
void* glutSolidCone(void);
void* glutSolidCube(void);
void* glutSolidDodecahedron(void);
void* glutSolidIcosahedron(void);
void* glutSolidOctahedron(void);
void* glutSolidSphere(void);
void* glutSolidTeapot(void);
void* glutSolidTetrahedron(void);
void* glutSolidTorus(void);
void* glutSpaceballButtonFunc(void);
void* glutSpaceballMotionFunc(void);
void* glutSpaceballRotateFunc(void);
void* glutSpecialFunc(void);
void* glutSpecialUpFunc(void);
void* glutStopVideoResizing(void);
void* glutStrokeCharacter(void);
void* glutStrokeLength(void);
void* glutStrokeWidth(void);
void* glutSurfaceTexture(void);
void* glutSwapBuffers(void);
void* glutTabletButtonFunc(void);
void* glutTabletMotionFunc(void);
void* glutTimerFunc(void);
void* glutUseLayer(void);
void* glutVideoPan(void);
void* glutVideoResize(void);
void* glutVideoResizeGet(void);
void* glutVisibilityFunc(void);
void* glutWMCloseFunc(void);
void* glutWarpPointer(void);
void* glutWindowStatusFunc(void);
void* glutWireCone(void);
void* glutWireCube(void);
void* glutWireDodecahedron(void);
void* glutWireIcosahedron(void);
void* glutWireOctahedron(void);
void* glutWireSphere(void);
void* glutWireTeapot(void);
void* glutWireTetrahedron(void);
void* glutWireTorus(void);
void* smapConfigureSphereMapMesh(void);
void* smapCreateSphereMap(void);
void* smapDestroySphereMap(void);
void* smapGenSphereMap(void);
void* smapGenSphereMapFromViewTexs(void);
void* smapGenSphereMapWithOneViewTex(void);
void* smapGenViewTex(void);
void* smapGenViewTexs(void);
void* smapGetContextData(void);
void* smapGetDrawViewFunc(void);
void* smapGetEye(void);
void* smapGetEyeVector(void);
void* smapGetFlags(void);
void* smapGetNearFar(void);
void* smapGetObject(void);
void* smapGetObjectVector(void);
void* smapGetPositionLightsFunc(void);
void* smapGetSphereMapOrigin(void);
void* smapGetSphereMapTexDim(void);
void* smapGetSphereMapTexObj(void);
void* smapGetUp(void);
void* smapGetUpVector(void);
void* smapGetViewOrigin(void);
void* smapGetViewTexDim(void);
void* smapGetViewTexObj(void);
void* smapGetViewTexObjs(void);
void* smapRvecToSt(void);
void* smapSetContextData(void);
void* smapSetDrawViewFunc(void);
void* smapSetEye(void);
void* smapSetEyeVector(void);
void* smapSetFlags(void);
void* smapSetNearFar(void);
void* smapSetObject(void);
void* smapSetObjectVector(void);
void* smapSetPositionLightsFunc(void);
void* smapSetSphereMapOrigin(void);
void* smapSetSphereMapTexDim(void);
void* smapSetSphereMapTexObj(void);
void* smapSetUp(void);
void* smapSetUpVector(void);
void* smapSetViewOrigin(void);
void* smapSetViewTexDim(void);
void* smapSetViewTexObj(void);
void* smapSetViewTexObjs(void);
void* smapStToRvec(void);

#endif
