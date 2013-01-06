In the early 1990s, I spent a lot of time playing netrek.

I played on some of the early INL teams.

I learned a lot about programming by playing, and by building cyborg players.

BSD-sockets, cryptographic signatures, network protocols, strategy, dogfighting, team play  -- netrek had a lot to offer (in the days of the pre-WWW internet).

In the mid-1990s, I played a lot of "Paradise" netrek: a variant which introduced some new strategy elements that I really liked (jumpships especially).

http://www.netrek.org/developer/design.html

----

On my Mac the config script happily finds the X11 includes, but (since it is looking for libX11.a) fails to find the libs:

(cd cow-client; ./configure --x-libraries=/usr/X11/lib)
