// Copyright (C) 2026 Lubos Dolezel
//
// Typecheck of the SceneKit Clang module and Swift API:
// basic scene and node creation.
// Build: swiftc -typecheck scenekit_import.swift -target x86_64-apple-macosx10.13 -sdk <SDK>

import Foundation
import SceneKit

func testSceneKit() {
    let scene = SCNScene()
    let node = SCNNode()
    _ = (scene, node)
}
testSceneKit()
