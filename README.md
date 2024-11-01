# pec-engine

Tl;dr - a KSP-like space simulator written in C++, uses n-body physics and it's not very fun (for the moment?). It only runs on Linux atm.

## What is this?

This is a little project I have been working on for a while. It is intended to be a [KSP-like](https://en.wikipedia.org/wiki/Kerbal_Space_Program) space simulator written from (almost) scratch in C++.
Instead of the [patched conic](https://en.wikipedia.org/wiki/Patched_conic_approximation) approach of KSP, it uses a quasi-[n-body](https://en.wikipedia.org/wiki/N-body_simulation) simulation, where all the major bodies' trajectories (planets, moons, etc) are calculated using patched conics and the rest of the objects' trajectories (spaceships, planes, ground vehicles, etc.) are simulated by applying the laws of classical mechanics. This is therefore not a full n-body simulation; only the major bodies influence the smaller objects, while the latter do not influence each other.
While this approach seemed conceptually easy, my naive enthusiasm has been repeatedly challenged by the realization of how utterly complex this problem is. 

## The engine

The project is written from scratch, although it uses some third-party libraries. I try to be rigorous with my codebase writing, but this means I have spent a considerable amount of time re-factoring stuff.

The engine is also significantly parallelized. The "logic/main" thread runs in parallel synchrony with the physics thread (they start together and wait for each other). The physics thread uses [Bullet](https://github.com/bulletphysics/bullet3) for the collisions and rigid body dynamics, and calculates the gravity forces on the objects. 
Since the logic thread might want to interact with the physics thread (for example, a rocket might want to apply a force on itself), the main thread has a series of command buffers to perform certain actions (apply forces, add/remove rigid bodies, change a motion state etc.). Meanwhile, the rendering thread runs unsynchronized and, possibly but not necessarily, at its own framerate. The logic thread uses a double-buffer system to communicate what to render with the rendering thread. The buffers contain shared pointers to the objects we want to render, avoiding this way accessing objects that might have been deleted in the main thread.

## Features

This is some of the stuff that I have done or maybe want to do. The images are not spectacular because I don't have a single good-looking 3D model, but they illustrate the currently implemented features.

- An editor, which works similarly to KPS's. The vessels are trees of parts (tanks, engines, etc.) to which the player can attach other parts or subtrees. It supports attaching multiple parts radially and multiple symmetrical attachments (if the part supports it). If the player attaches to symmetrical sub-trees and the symmetrical attachment option is on, the part being attached will be cloned to the other symmetrical sub-trees. It also has a primitive staging system, which supports, among other actions, detaching parts of the rocket during gameplay.

<p align="center">
  <img src="https://i.imgur.com/9jEMq8Q.png" alt="editor" width="600"/>
</p>

- A planetary view of the planetary system, which includes a kinda dumb trajectory predictor that needs a bit of work. The predictor uses a fixed time-step which causes a lot of trouble.

<p align="center">
  <img src="https://i.imgur.com/idLDucC.png" alt="planet view" width="400"/>
  <img src="https://i.imgur.com/bHR4FoW.png" alt="planet view" width="400"/>
</p>

more stuff coming soon

