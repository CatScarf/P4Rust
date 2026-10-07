# Vendor notices

Perforce API sources originate from the official 2026.1.3062361 source distribution at <https://ftp.perforce.com/perforce/r26.1/bin.tools/p4source.tgz>. The complete source license is retained in `LICENSE` and copied into every packaged resource crate. Source copyright notices remain intact.

Jam 2.6 originates from <https://swarm.workshop.perforce.com/downloads/guest/perforce_software/jam/jam-2.6.zip>. Its source tree includes the vendor license. Build-time modifications to vendor rules and the Jam command limit are marked in the extracted cache.

`sources.json` records exact upstream and source-file checksums. OpenSSL uses the pinned `openssl-src` dependency and its Apache 2.0 license, retained at `native/OPENSSL-LICENSE.txt` and included in every resource crate.