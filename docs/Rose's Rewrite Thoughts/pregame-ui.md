# Why did we decide to use multiple binaries?

So this is the question I initially had when re-evaluating Phoenix's CLI goals.
We wanted it to be like minecraft. A similar and more standard UI means getting
things up and running is more intuitive for users. But that's just the start.
Solutions such as these can't be made so short sighted. We should have evaluated
the pros and cons both before choosing this. Admittedly, I was too inexperienced
to really understand the implications of using a split Server & Client UI.
So was everyone else on the team? That's why I'm taking this time to use my
current experience and rewrite this. Before I move on to my conclusion, lets
take a look at all the options.

 1. *Split UI*: one executable for the Server, and one for the Client.
 2. *Combined UI*: a single executable containing both the Server and Client.
 3. *Split UI w/ Shared Library*: two executables and a library with shared code.

These were the three options I initially came up with when I was thinking
through the problem. I wasn't a part of the development team in the project's
infancy. It may only be in version 0.2 at this point; but that's a three year
development. The point is, I don't know what the original discussion was
for this. All I know is, the conclusion reached seems to have been
that the project should be like MC.

##Features

### Split UI
**Pros**
 1. Dedicated, distributed binaries with less bloat for server host systems.
 2. A more direct CLI for servers and clients alike.

**Cons**
 1. Introduces linking challenges if you want to have the CLI be standalone.
 2. Otherwise you have to bundle both whenever you distribute the game, and
    if one goes missing, then there's a fatal error and it becomes an
    obstruction for users.
 3. Introduces redundant source code that could otherwise be condensed.
 4. Complexity within the code base in general.

### Split UI w/ Shared Library
**Pros**
 1. Same as Split UI
 2. Removes some linking challenges by fully embracing distributed code.
    In this state, it's expected the binaries will be distributed by an external
    package management system.

**Cons**
 1. Complexity within the code base is still pervasive.

### Combined UI
**Pros**
 1. It eliminates code complexity, and redundancy.
 2. There's less to distribute.
 3. Using advanced CLI parsers such as argparse, concise CLI can still be
    attained.
 4. Some of the GUI code in the two other solutions can be reused to for
    both the client and server. Such as the initial startup page. Ex:
		Users could start the same way for both hosting and playing, there could
		be a menu option for server hosting standalone.

**Cons**
 None that I can think of.

## Conclusion

I personally don't think the above did a good job of conveying my point, but
I don't care. I'm tired and it's getting late, my motivation is worn for the
day.

I'm taking lessons from the [*Rust Language*][rustlang] ecosystem. One of the
goals in their UI was to embody all the facets of modern programming in a single
executable. This single executable hosts not only the languages build system, but
also the linter, and a standard documentation engine. While there is a split
package manager, **Cargo**, there's a good reason. Package management has
nothing to do with the code itself. It's an entirely different system.
While you could say that having a good package manager and ecosystem is
essential for a good language; it's not 100% necessary. Each part can function
independently.

What I'm getting at with this Server and Client split is, they're inherently
linked processes. You can't have a server without some kind of client to
access it. And while a split scheme can have its benefits, ie. the web, that
doesn't necessarily mean that it's best everywhere. Here, the client and server
are 100% codependent. The game wouldn't be complete without the server or
the client. So therefore, they should both exist within the same executable.



[rustlang]: https://www.rust-lang.org/
