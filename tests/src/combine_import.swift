// Copyright (C) 2026 Lubos Dolezel
//
// Typecheck and verification of the Combine framework public API:
// Publishers, Subscribers, Subjects, AnyCancellable, and ObservableObject / @Published.
//
// Build: swiftc -typecheck combine_import.swift -target x86_64-apple-macosx26.0 -sdk <SDK>
import Combine
import Foundation

var failures = 0
func check(_ condition: Bool, _ label: String) {
    if !condition {
        print("FAIL: \(label)")
        failures += 1
    }
}

// 1. Just publisher and sink
var receivedJust = 0
var justFinished = false
var cancellables = Set<AnyCancellable>()

Just(100)
    .sink(receiveCompletion: { completion in
        if case .finished = completion {
            justFinished = true
        }
    }, receiveValue: { val in
        receivedJust = val
    })
    .store(in: &cancellables)

check(receivedJust == 100, "Just delivers value")
check(justFinished, "Just finishes")

// 2. PassthroughSubject
let passthrough = PassthroughSubject<String, Never>()
var receivedStrings: [String] = []

passthrough
    .sink { val in
        receivedStrings.append(val)
    }
    .store(in: &cancellables)

passthrough.send("hello")
passthrough.send("combine")
check(receivedStrings == ["hello", "combine"], "PassthroughSubject sends values")

// 3. CurrentValueSubject
let current = CurrentValueSubject<Int, Never>(1)
var currentValues: [Int] = []

current
    .sink { val in
        currentValues.append(val)
    }
    .store(in: &cancellables)

current.send(2)
current.value = 3
check(currentValues == [1, 2, 3], "CurrentValueSubject replays initial and receives updates")
check(current.value == 3, "CurrentValueSubject.value getter")

// 4. Operators: filter, map
var transformed: [Int] = []
[1, 2, 3, 4, 5, 6].publisher
    .filter { $0 % 2 == 0 }
    .map { $0 * 10 }
    .sink { val in
        transformed.append(val)
    }
    .store(in: &cancellables)

check(transformed == [20, 40, 60], "filter and map operate correctly")

// 5. ObservableObject and @Published
final class TestViewModel: ObservableObject {
    @Published var title: String = "initial"
    @Published var score: Int = 0
}

let vm = TestViewModel()
var willChangeFired = 0
vm.objectWillChange
    .sink { _ in
        willChangeFired += 1
    }
    .store(in: &cancellables)

var publishedTitles: [String] = []
vm.$title
    .sink { val in
        publishedTitles.append(val)
    }
    .store(in: &cancellables)

vm.title = "second"
vm.score = 42

check(publishedTitles == ["initial", "second"], "@Published projected value publisher")
check(willChangeFired == 2, "objectWillChange fires on property mutations")
check(vm.score == 42, "@Published stored value")

// 6. AnyCancellable cancel
let cancelSubject = PassthroughSubject<Int, Never>()
var valuesBeforeCancel: [Int] = []
let token = cancelSubject.sink { val in
    valuesBeforeCancel.append(val)
}
cancelSubject.send(1)
token.cancel()
cancelSubject.send(2)
check(valuesBeforeCancel == [1], "AnyCancellable stops emissions after cancel")

// 7. Fail
enum SampleError: Error {
    case testFailure
}

var failureReceived = false
Fail<Int, SampleError>(error: .testFailure)
    .sink(receiveCompletion: { completion in
        if case .failure(.testFailure) = completion {
            failureReceived = true
        }
    }, receiveValue: { _ in })
    .store(in: &cancellables)

check(failureReceived, "Fail delivers expected error completion")

if failures == 0 {
    print("ALL PASSED")
} else {
    exit(1)
}
