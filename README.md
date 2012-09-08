Group Allocation
================

Problem statement: Assign a set of persons to a set of groups. Each person provides a certain preference to which group it wants to belong. Maximize the global satisfaction of the participants!

This is the implementation of a genetic algorithm that solves this allocation problem.

**Input format**

    x y
    preference preference ...
    preference preference ...
    ...        ...        ...

where ``x`` = persons(per row) and ``y``= groups(per column)