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


#include <GLUT/GLUT.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* __glutGetFCB(void)
{
    if (verbose) puts("STUB: __glutGetFCB called");
    return NULL;
}

void* __glutSetFCB(void)
{
    if (verbose) puts("STUB: __glutSetFCB called");
    return NULL;
}

void* gleCreateGC(void)
{
    if (verbose) puts("STUB: gleCreateGC called");
    return NULL;
}

void* gleExtrusion(void)
{
    if (verbose) puts("STUB: gleExtrusion called");
    return NULL;
}

void* gleGetJoinStyle(void)
{
    if (verbose) puts("STUB: gleGetJoinStyle called");
    return NULL;
}

void* gleGetNumSlices(void)
{
    if (verbose) puts("STUB: gleGetNumSlices called");
    return NULL;
}

void* gleHelicoid(void)
{
    if (verbose) puts("STUB: gleHelicoid called");
    return NULL;
}

void* gleLathe(void)
{
    if (verbose) puts("STUB: gleLathe called");
    return NULL;
}

void* glePolyCone(void)
{
    if (verbose) puts("STUB: glePolyCone called");
    return NULL;
}

void* glePolyCylinder(void)
{
    if (verbose) puts("STUB: glePolyCylinder called");
    return NULL;
}

void* gleScrew(void)
{
    if (verbose) puts("STUB: gleScrew called");
    return NULL;
}

void* gleSetJoinStyle(void)
{
    if (verbose) puts("STUB: gleSetJoinStyle called");
    return NULL;
}

void* gleSetNumSlices(void)
{
    if (verbose) puts("STUB: gleSetNumSlices called");
    return NULL;
}

void* gleSpiral(void)
{
    if (verbose) puts("STUB: gleSpiral called");
    return NULL;
}

void* gleSuperExtrusion(void)
{
    if (verbose) puts("STUB: gleSuperExtrusion called");
    return NULL;
}

void* gleTextureMode(void)
{
    if (verbose) puts("STUB: gleTextureMode called");
    return NULL;
}

void* gleToroid(void)
{
    if (verbose) puts("STUB: gleToroid called");
    return NULL;
}

void* gleTwistExtrusion(void)
{
    if (verbose) puts("STUB: gleTwistExtrusion called");
    return NULL;
}

void* glutAddMenuEntry(void)
{
    if (verbose) puts("STUB: glutAddMenuEntry called");
    return NULL;
}

void* glutAddSubMenu(void)
{
    if (verbose) puts("STUB: glutAddSubMenu called");
    return NULL;
}

void* glutAttachMenu(void)
{
    if (verbose) puts("STUB: glutAttachMenu called");
    return NULL;
}

void* glutBitmapCharacter(void)
{
    if (verbose) puts("STUB: glutBitmapCharacter called");
    return NULL;
}

void* glutBitmapLength(void)
{
    if (verbose) puts("STUB: glutBitmapLength called");
    return NULL;
}

void* glutBitmapWidth(void)
{
    if (verbose) puts("STUB: glutBitmapWidth called");
    return NULL;
}

void* glutButtonBoxFunc(void)
{
    if (verbose) puts("STUB: glutButtonBoxFunc called");
    return NULL;
}

void* glutChangeToMenuEntry(void)
{
    if (verbose) puts("STUB: glutChangeToMenuEntry called");
    return NULL;
}

void* glutChangeToSubMenu(void)
{
    if (verbose) puts("STUB: glutChangeToSubMenu called");
    return NULL;
}

void* glutCheckLoop(void)
{
    if (verbose) puts("STUB: glutCheckLoop called");
    return NULL;
}

void* glutCopyColormap(void)
{
    if (verbose) puts("STUB: glutCopyColormap called");
    return NULL;
}

void* glutCreateMenu(void)
{
    if (verbose) puts("STUB: glutCreateMenu called");
    return NULL;
}

void* glutCreateSubWindow(void)
{
    if (verbose) puts("STUB: glutCreateSubWindow called");
    return NULL;
}

void* glutCreateWindow(void)
{
    if (verbose) puts("STUB: glutCreateWindow called");
    return NULL;
}

void* glutDestroyMenu(void)
{
    if (verbose) puts("STUB: glutDestroyMenu called");
    return NULL;
}

void* glutDestroyWindow(void)
{
    if (verbose) puts("STUB: glutDestroyWindow called");
    return NULL;
}

void* glutDetachMenu(void)
{
    if (verbose) puts("STUB: glutDetachMenu called");
    return NULL;
}

void* glutDeviceGet(void)
{
    if (verbose) puts("STUB: glutDeviceGet called");
    return NULL;
}

void* glutDialsFunc(void)
{
    if (verbose) puts("STUB: glutDialsFunc called");
    return NULL;
}

void* glutDisplayFunc(void)
{
    if (verbose) puts("STUB: glutDisplayFunc called");
    return NULL;
}

void* glutEnterGameMode(void)
{
    if (verbose) puts("STUB: glutEnterGameMode called");
    return NULL;
}

void* glutEntryFunc(void)
{
    if (verbose) puts("STUB: glutEntryFunc called");
    return NULL;
}

void* glutEstablishOverlay(void)
{
    if (verbose) puts("STUB: glutEstablishOverlay called");
    return NULL;
}

void* glutExtensionSupported(void)
{
    if (verbose) puts("STUB: glutExtensionSupported called");
    return NULL;
}

void* glutForceJoystickFunc(void)
{
    if (verbose) puts("STUB: glutForceJoystickFunc called");
    return NULL;
}

void* glutFullScreen(void)
{
    if (verbose) puts("STUB: glutFullScreen called");
    return NULL;
}

void* glutGameModeGet(void)
{
    if (verbose) puts("STUB: glutGameModeGet called");
    return NULL;
}

void* glutGameModeString(void)
{
    if (verbose) puts("STUB: glutGameModeString called");
    return NULL;
}

void* glutGet(void)
{
    if (verbose) puts("STUB: glutGet called");
    return NULL;
}

void* glutGetColor(void)
{
    if (verbose) puts("STUB: glutGetColor called");
    return NULL;
}

void* glutGetMenu(void)
{
    if (verbose) puts("STUB: glutGetMenu called");
    return NULL;
}

void* glutGetModifiers(void)
{
    if (verbose) puts("STUB: glutGetModifiers called");
    return NULL;
}

void* glutGetProcAddress(void)
{
    if (verbose) puts("STUB: glutGetProcAddress called");
    return NULL;
}

void* glutGetWindow(void)
{
    if (verbose) puts("STUB: glutGetWindow called");
    return NULL;
}

void* glutHideOverlay(void)
{
    if (verbose) puts("STUB: glutHideOverlay called");
    return NULL;
}

void* glutHideWindow(void)
{
    if (verbose) puts("STUB: glutHideWindow called");
    return NULL;
}

void* glutIconifyWindow(void)
{
    if (verbose) puts("STUB: glutIconifyWindow called");
    return NULL;
}

void* glutIdleFunc(void)
{
    if (verbose) puts("STUB: glutIdleFunc called");
    return NULL;
}

void* glutIgnoreKeyRepeat(void)
{
    if (verbose) puts("STUB: glutIgnoreKeyRepeat called");
    return NULL;
}

void* glutInit(void)
{
    if (verbose) puts("STUB: glutInit called");
    return NULL;
}

void* glutInitDisplayMode(void)
{
    if (verbose) puts("STUB: glutInitDisplayMode called");
    return NULL;
}

void* glutInitDisplayString(void)
{
    if (verbose) puts("STUB: glutInitDisplayString called");
    return NULL;
}

void* glutInitWindowPosition(void)
{
    if (verbose) puts("STUB: glutInitWindowPosition called");
    return NULL;
}

void* glutInitWindowSize(void)
{
    if (verbose) puts("STUB: glutInitWindowSize called");
    return NULL;
}

void* glutJoystickFunc(void)
{
    if (verbose) puts("STUB: glutJoystickFunc called");
    return NULL;
}

void* glutKeyboardFunc(void)
{
    if (verbose) puts("STUB: glutKeyboardFunc called");
    return NULL;
}

void* glutKeyboardUpFunc(void)
{
    if (verbose) puts("STUB: glutKeyboardUpFunc called");
    return NULL;
}

void* glutLayerGet(void)
{
    if (verbose) puts("STUB: glutLayerGet called");
    return NULL;
}

void* glutLeaveGameMode(void)
{
    if (verbose) puts("STUB: glutLeaveGameMode called");
    return NULL;
}

void* glutMainLoop(void)
{
    if (verbose) puts("STUB: glutMainLoop called");
    return NULL;
}

void* glutMenuStateFunc(void)
{
    if (verbose) puts("STUB: glutMenuStateFunc called");
    return NULL;
}

void* glutMenuStatusFunc(void)
{
    if (verbose) puts("STUB: glutMenuStatusFunc called");
    return NULL;
}

void* glutMotionFunc(void)
{
    if (verbose) puts("STUB: glutMotionFunc called");
    return NULL;
}

void* glutMouseFunc(void)
{
    if (verbose) puts("STUB: glutMouseFunc called");
    return NULL;
}

void* glutOverlayDisplayFunc(void)
{
    if (verbose) puts("STUB: glutOverlayDisplayFunc called");
    return NULL;
}

void* glutPassiveMotionFunc(void)
{
    if (verbose) puts("STUB: glutPassiveMotionFunc called");
    return NULL;
}

void* glutPopWindow(void)
{
    if (verbose) puts("STUB: glutPopWindow called");
    return NULL;
}

void* glutPositionWindow(void)
{
    if (verbose) puts("STUB: glutPositionWindow called");
    return NULL;
}

void* glutPostOverlayRedisplay(void)
{
    if (verbose) puts("STUB: glutPostOverlayRedisplay called");
    return NULL;
}

void* glutPostRedisplay(void)
{
    if (verbose) puts("STUB: glutPostRedisplay called");
    return NULL;
}

void* glutPostWindowOverlayRedisplay(void)
{
    if (verbose) puts("STUB: glutPostWindowOverlayRedisplay called");
    return NULL;
}

void* glutPostWindowRedisplay(void)
{
    if (verbose) puts("STUB: glutPostWindowRedisplay called");
    return NULL;
}

void* glutPushWindow(void)
{
    if (verbose) puts("STUB: glutPushWindow called");
    return NULL;
}

void* glutRemoveMenuItem(void)
{
    if (verbose) puts("STUB: glutRemoveMenuItem called");
    return NULL;
}

void* glutRemoveOverlay(void)
{
    if (verbose) puts("STUB: glutRemoveOverlay called");
    return NULL;
}

void* glutReportErrors(void)
{
    if (verbose) puts("STUB: glutReportErrors called");
    return NULL;
}

void* glutReshapeFunc(void)
{
    if (verbose) puts("STUB: glutReshapeFunc called");
    return NULL;
}

void* glutReshapeWindow(void)
{
    if (verbose) puts("STUB: glutReshapeWindow called");
    return NULL;
}

void* glutSetColor(void)
{
    if (verbose) puts("STUB: glutSetColor called");
    return NULL;
}

void* glutSetCursor(void)
{
    if (verbose) puts("STUB: glutSetCursor called");
    return NULL;
}

void* glutSetIconTitle(void)
{
    if (verbose) puts("STUB: glutSetIconTitle called");
    return NULL;
}

void* glutSetKeyRepeat(void)
{
    if (verbose) puts("STUB: glutSetKeyRepeat called");
    return NULL;
}

void* glutSetMenu(void)
{
    if (verbose) puts("STUB: glutSetMenu called");
    return NULL;
}

void* glutSetWindow(void)
{
    if (verbose) puts("STUB: glutSetWindow called");
    return NULL;
}

void* glutSetWindowTitle(void)
{
    if (verbose) puts("STUB: glutSetWindowTitle called");
    return NULL;
}

void* glutSetupVideoResizing(void)
{
    if (verbose) puts("STUB: glutSetupVideoResizing called");
    return NULL;
}

void* glutShowOverlay(void)
{
    if (verbose) puts("STUB: glutShowOverlay called");
    return NULL;
}

void* glutShowWindow(void)
{
    if (verbose) puts("STUB: glutShowWindow called");
    return NULL;
}

void* glutSolidCone(void)
{
    if (verbose) puts("STUB: glutSolidCone called");
    return NULL;
}

void* glutSolidCube(void)
{
    if (verbose) puts("STUB: glutSolidCube called");
    return NULL;
}

void* glutSolidDodecahedron(void)
{
    if (verbose) puts("STUB: glutSolidDodecahedron called");
    return NULL;
}

void* glutSolidIcosahedron(void)
{
    if (verbose) puts("STUB: glutSolidIcosahedron called");
    return NULL;
}

void* glutSolidOctahedron(void)
{
    if (verbose) puts("STUB: glutSolidOctahedron called");
    return NULL;
}

void* glutSolidSphere(void)
{
    if (verbose) puts("STUB: glutSolidSphere called");
    return NULL;
}

void* glutSolidTeapot(void)
{
    if (verbose) puts("STUB: glutSolidTeapot called");
    return NULL;
}

void* glutSolidTetrahedron(void)
{
    if (verbose) puts("STUB: glutSolidTetrahedron called");
    return NULL;
}

void* glutSolidTorus(void)
{
    if (verbose) puts("STUB: glutSolidTorus called");
    return NULL;
}

void* glutSpaceballButtonFunc(void)
{
    if (verbose) puts("STUB: glutSpaceballButtonFunc called");
    return NULL;
}

void* glutSpaceballMotionFunc(void)
{
    if (verbose) puts("STUB: glutSpaceballMotionFunc called");
    return NULL;
}

void* glutSpaceballRotateFunc(void)
{
    if (verbose) puts("STUB: glutSpaceballRotateFunc called");
    return NULL;
}

void* glutSpecialFunc(void)
{
    if (verbose) puts("STUB: glutSpecialFunc called");
    return NULL;
}

void* glutSpecialUpFunc(void)
{
    if (verbose) puts("STUB: glutSpecialUpFunc called");
    return NULL;
}

void* glutStopVideoResizing(void)
{
    if (verbose) puts("STUB: glutStopVideoResizing called");
    return NULL;
}

void* glutStrokeCharacter(void)
{
    if (verbose) puts("STUB: glutStrokeCharacter called");
    return NULL;
}

void* glutStrokeLength(void)
{
    if (verbose) puts("STUB: glutStrokeLength called");
    return NULL;
}

void* glutStrokeWidth(void)
{
    if (verbose) puts("STUB: glutStrokeWidth called");
    return NULL;
}

void* glutSurfaceTexture(void)
{
    if (verbose) puts("STUB: glutSurfaceTexture called");
    return NULL;
}

void* glutSwapBuffers(void)
{
    if (verbose) puts("STUB: glutSwapBuffers called");
    return NULL;
}

void* glutTabletButtonFunc(void)
{
    if (verbose) puts("STUB: glutTabletButtonFunc called");
    return NULL;
}

void* glutTabletMotionFunc(void)
{
    if (verbose) puts("STUB: glutTabletMotionFunc called");
    return NULL;
}

void* glutTimerFunc(void)
{
    if (verbose) puts("STUB: glutTimerFunc called");
    return NULL;
}

void* glutUseLayer(void)
{
    if (verbose) puts("STUB: glutUseLayer called");
    return NULL;
}

void* glutVideoPan(void)
{
    if (verbose) puts("STUB: glutVideoPan called");
    return NULL;
}

void* glutVideoResize(void)
{
    if (verbose) puts("STUB: glutVideoResize called");
    return NULL;
}

void* glutVideoResizeGet(void)
{
    if (verbose) puts("STUB: glutVideoResizeGet called");
    return NULL;
}

void* glutVisibilityFunc(void)
{
    if (verbose) puts("STUB: glutVisibilityFunc called");
    return NULL;
}

void* glutWMCloseFunc(void)
{
    if (verbose) puts("STUB: glutWMCloseFunc called");
    return NULL;
}

void* glutWarpPointer(void)
{
    if (verbose) puts("STUB: glutWarpPointer called");
    return NULL;
}

void* glutWindowStatusFunc(void)
{
    if (verbose) puts("STUB: glutWindowStatusFunc called");
    return NULL;
}

void* glutWireCone(void)
{
    if (verbose) puts("STUB: glutWireCone called");
    return NULL;
}

void* glutWireCube(void)
{
    if (verbose) puts("STUB: glutWireCube called");
    return NULL;
}

void* glutWireDodecahedron(void)
{
    if (verbose) puts("STUB: glutWireDodecahedron called");
    return NULL;
}

void* glutWireIcosahedron(void)
{
    if (verbose) puts("STUB: glutWireIcosahedron called");
    return NULL;
}

void* glutWireOctahedron(void)
{
    if (verbose) puts("STUB: glutWireOctahedron called");
    return NULL;
}

void* glutWireSphere(void)
{
    if (verbose) puts("STUB: glutWireSphere called");
    return NULL;
}

void* glutWireTeapot(void)
{
    if (verbose) puts("STUB: glutWireTeapot called");
    return NULL;
}

void* glutWireTetrahedron(void)
{
    if (verbose) puts("STUB: glutWireTetrahedron called");
    return NULL;
}

void* glutWireTorus(void)
{
    if (verbose) puts("STUB: glutWireTorus called");
    return NULL;
}

void* smapConfigureSphereMapMesh(void)
{
    if (verbose) puts("STUB: smapConfigureSphereMapMesh called");
    return NULL;
}

void* smapCreateSphereMap(void)
{
    if (verbose) puts("STUB: smapCreateSphereMap called");
    return NULL;
}

void* smapDestroySphereMap(void)
{
    if (verbose) puts("STUB: smapDestroySphereMap called");
    return NULL;
}

void* smapGenSphereMap(void)
{
    if (verbose) puts("STUB: smapGenSphereMap called");
    return NULL;
}

void* smapGenSphereMapFromViewTexs(void)
{
    if (verbose) puts("STUB: smapGenSphereMapFromViewTexs called");
    return NULL;
}

void* smapGenSphereMapWithOneViewTex(void)
{
    if (verbose) puts("STUB: smapGenSphereMapWithOneViewTex called");
    return NULL;
}

void* smapGenViewTex(void)
{
    if (verbose) puts("STUB: smapGenViewTex called");
    return NULL;
}

void* smapGenViewTexs(void)
{
    if (verbose) puts("STUB: smapGenViewTexs called");
    return NULL;
}

void* smapGetContextData(void)
{
    if (verbose) puts("STUB: smapGetContextData called");
    return NULL;
}

void* smapGetDrawViewFunc(void)
{
    if (verbose) puts("STUB: smapGetDrawViewFunc called");
    return NULL;
}

void* smapGetEye(void)
{
    if (verbose) puts("STUB: smapGetEye called");
    return NULL;
}

void* smapGetEyeVector(void)
{
    if (verbose) puts("STUB: smapGetEyeVector called");
    return NULL;
}

void* smapGetFlags(void)
{
    if (verbose) puts("STUB: smapGetFlags called");
    return NULL;
}

void* smapGetNearFar(void)
{
    if (verbose) puts("STUB: smapGetNearFar called");
    return NULL;
}

void* smapGetObject(void)
{
    if (verbose) puts("STUB: smapGetObject called");
    return NULL;
}

void* smapGetObjectVector(void)
{
    if (verbose) puts("STUB: smapGetObjectVector called");
    return NULL;
}

void* smapGetPositionLightsFunc(void)
{
    if (verbose) puts("STUB: smapGetPositionLightsFunc called");
    return NULL;
}

void* smapGetSphereMapOrigin(void)
{
    if (verbose) puts("STUB: smapGetSphereMapOrigin called");
    return NULL;
}

void* smapGetSphereMapTexDim(void)
{
    if (verbose) puts("STUB: smapGetSphereMapTexDim called");
    return NULL;
}

void* smapGetSphereMapTexObj(void)
{
    if (verbose) puts("STUB: smapGetSphereMapTexObj called");
    return NULL;
}

void* smapGetUp(void)
{
    if (verbose) puts("STUB: smapGetUp called");
    return NULL;
}

void* smapGetUpVector(void)
{
    if (verbose) puts("STUB: smapGetUpVector called");
    return NULL;
}

void* smapGetViewOrigin(void)
{
    if (verbose) puts("STUB: smapGetViewOrigin called");
    return NULL;
}

void* smapGetViewTexDim(void)
{
    if (verbose) puts("STUB: smapGetViewTexDim called");
    return NULL;
}

void* smapGetViewTexObj(void)
{
    if (verbose) puts("STUB: smapGetViewTexObj called");
    return NULL;
}

void* smapGetViewTexObjs(void)
{
    if (verbose) puts("STUB: smapGetViewTexObjs called");
    return NULL;
}

void* smapRvecToSt(void)
{
    if (verbose) puts("STUB: smapRvecToSt called");
    return NULL;
}

void* smapSetContextData(void)
{
    if (verbose) puts("STUB: smapSetContextData called");
    return NULL;
}

void* smapSetDrawViewFunc(void)
{
    if (verbose) puts("STUB: smapSetDrawViewFunc called");
    return NULL;
}

void* smapSetEye(void)
{
    if (verbose) puts("STUB: smapSetEye called");
    return NULL;
}

void* smapSetEyeVector(void)
{
    if (verbose) puts("STUB: smapSetEyeVector called");
    return NULL;
}

void* smapSetFlags(void)
{
    if (verbose) puts("STUB: smapSetFlags called");
    return NULL;
}

void* smapSetNearFar(void)
{
    if (verbose) puts("STUB: smapSetNearFar called");
    return NULL;
}

void* smapSetObject(void)
{
    if (verbose) puts("STUB: smapSetObject called");
    return NULL;
}

void* smapSetObjectVector(void)
{
    if (verbose) puts("STUB: smapSetObjectVector called");
    return NULL;
}

void* smapSetPositionLightsFunc(void)
{
    if (verbose) puts("STUB: smapSetPositionLightsFunc called");
    return NULL;
}

void* smapSetSphereMapOrigin(void)
{
    if (verbose) puts("STUB: smapSetSphereMapOrigin called");
    return NULL;
}

void* smapSetSphereMapTexDim(void)
{
    if (verbose) puts("STUB: smapSetSphereMapTexDim called");
    return NULL;
}

void* smapSetSphereMapTexObj(void)
{
    if (verbose) puts("STUB: smapSetSphereMapTexObj called");
    return NULL;
}

void* smapSetUp(void)
{
    if (verbose) puts("STUB: smapSetUp called");
    return NULL;
}

void* smapSetUpVector(void)
{
    if (verbose) puts("STUB: smapSetUpVector called");
    return NULL;
}

void* smapSetViewOrigin(void)
{
    if (verbose) puts("STUB: smapSetViewOrigin called");
    return NULL;
}

void* smapSetViewTexDim(void)
{
    if (verbose) puts("STUB: smapSetViewTexDim called");
    return NULL;
}

void* smapSetViewTexObj(void)
{
    if (verbose) puts("STUB: smapSetViewTexObj called");
    return NULL;
}

void* smapSetViewTexObjs(void)
{
    if (verbose) puts("STUB: smapSetViewTexObjs called");
    return NULL;
}

void* smapStToRvec(void)
{
    if (verbose) puts("STUB: smapStToRvec called");
    return NULL;
}
