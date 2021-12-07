# team10
This commit is our submission for M4.

### Notes

- The game doesn't compile on Linux. Since we're using Mac and Windows only we didn't know it was a requirement to compile on Linux. Since the M3 feedback took a long time to reach us, we didn't have time to fix it even though we tried.

### Graphic assets
* wall.png: <div>Icons made by <a href="https://www.flaticon.com/authors/smashicons" title="Smashicons">Smashicons</a> from <a href="https://www.flaticon.com/" title="Flaticon">www.flaticon.com</a></div>
* enemy.png: <div>Icons made by <a href="https://www.freepik.com" title="Freepik">Freepik</a> from <a href="https://www.flaticon.com/" title="Flaticon">www.flaticon.com</a></div>
* drone.png:  <div>Icons made by <a href="https://www.flaticon.com/authors/smashicons" title="Smashicons">Smashicons</a> from <a href="https://www.flaticon.com/" title="Flaticon">www.flaticon.com</a></div>
* Minotaur_sprite_sheet.png:  https://elthen.itch.io/2d-pixel-art-minotaur-sprites
* background_moon.png: https://toppng.com/moon-PNG-free-PNG-Images_25446
* background_satellite.png: https://www.pngaaa.com/detail/1865660
* cutscene_drone.png | cutscene_drone.png_laughing.png | cutscene_drone.png_sad.png | cutscene_minotaur.png: Varalee Chinsomboon - used with permission
* cutscene_minotaur_rtx_off.png | cutscene_drone_rtx_off.png: Andrew Tsai
* xxx_normal_map.png generated from https://cpetry.github.io/NormalMap-Online/

### Fonts
* all: https://www.fontsquirrel.com/fonts/list/popular

### Libraries
* FreeType: https://sourceforge.net/projects/freetype/

### Audio

* Music by <a href="/users/juliush-3921568/?tab=audio&amp;utm_source=link-attribution&amp;utm_medium=referral&amp;utm_campaign=audio&amp;utm_content=4238">JuliusH</a> from <a href="https://pixabay.com/?utm_source=link-attribution&amp;utm_medium=referral&amp;utm_campaign=music&amp;utm_content=4238">Pixabay</a>

* Death sound: http://freesoundeffect.net/sound/mythical-beast-minotaur-sound-effect

* Power up sound: https://www.fesliyanstudios.com/royalty-free-sound-effects-download/spells-and-power-ups-217

* horse_snort.wav: https://freesound.org/people/ERH/sounds/32044/

* drone_stupid_boy.wav | drone_were_it_only_so_easy.wav : made with - Voice changer: https://voicechanger.io/ || Audio editor: https://twistedwave.com/online

* Other sound effects from <a href="https://pixabay.com/music/?utm_source=link-attribution&amp;utm_medium=referral&amp;utm_campaign=music&amp;utm_content=6185">Pixabay</a>

### Game Balance
* A theoretical analysis was performed in order to balance our game, where we altered the number of required keys to escape, the number of enemies, and the number of items, specifically.
* Initially, our game required the player to find a single key in order to escape at any given level, and per level progression the map size grows by 1.2 times, the number of enemies multiplied by 1.5, and the number of items are doubled.
* Also initially, the number of keys that spawn double on every level progression and the number of items double on every level progression.

* After reviewing the relative strengths of the entities in our game, we came up with a point system to measure the relative difficulty of our game by comparing the quantities of entities that work against the player 
and those that help the player succeed. A model equation was created as follows where x represents the phase/level progression:

Standardized values of entities in the games that impact player succcess:
- negative impact of enemies on player success : 0.3 (our enemies are fairly weak at this stage and would ideally have this value increase over time in the future)
- average positive impact of items on player success : 0.6

Point score for entities working against the player:
Points = Effect of map size increase + (Standardized Enemy Strength)*Number of Enemies + Number of Keys Required
Points = 1.2x + 0.3*(1.5x) + 1

Point score for entities that help the player:
Points = (Standardized Avg Item Strength)*Number of Items + Number of Health Points + Number of Keys Spawned
Points = 0.6*(2x) + 3 + 2x

When these two equations are plotted side-by-side, the blue curve represents the entities that help the player and the red curve is representative of the entities that work against the player.
It is clear that at these levels, the game is far too easy for the player and our goal is to move those lines closer together in order to balance the game.
Link to Graph Image Before Balancing:
https://imgur.com/VQuKQSP

After manipulation of multiple values, testing out new plots, and also seeing if these decisions fit with the current game progression, we decided to increase the number of enemies that spawn on every level progression, and a major
change was that we changed the number of required keys to exit the maze to scale linearly with the phase level as opposed to setting it constant at 1. This makes the game significantly harder but still not overly hard as will be shown in the next plot.

New Point score for entities working against the player:
Points = Effect of map size increase + (Standardized Enemy Strength)*Number of Enemies + Number of Keys Required
Points = 1.2x + 0.3*(2x) + (x+1)
Link to Graph Image After Balancing:
https://imgur.com/gVR8RQ4

As you can see, the game is much more balanced after this theoretical analysis and hope that it makes a signficant impact in the enjoyment of the game for our players!
Summary Changelog:
- altered constant number required of keys from 1 to a linearly increasing number dependent on phase level (x+1)
- altered the number of enemies multiplier per phase level from 1.5 to 2 in order make the game more difficult for the player
Other changes tested:
- map size increase > 1.2 per level; however, this made a significant impact on the difficulty of the game and the size of the maps got too large too fast, so we decided to keep it at this level for performance and overall gameplay
- decrease the number of items multiplier; however, this made the higher levels extremely difficult as navigating through extremely large mazes became significantly harder with having less items especially the wall-breaker




