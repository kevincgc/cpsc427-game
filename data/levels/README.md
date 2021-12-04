Each level should be a folder.
There should be a `level.yml` file inside, following the format:

```yaml
name: Testing Map 1         # REQUIRED | this is a human-readable name
type: premade               # REQUIRED | either 'premade'|'procedural'

# OPTIONAL | every enemy is optional, as well as the 'enemies' property. If no enemies
# are specified then no enemies will spawn.
enemies:
    spikes: 2               # will spawn exactly 2 of this enemy
    drones: [1, 2]          # will spawn between 1 and 2 (inclusive) of this enemy

# OPTIONAL | every item is optional, as well as the 'items' property. If no items
# are specified then no items will spawn.
items:
    extra_lives: [1, 2]     # will spawn between 1 and 2 (inclusive) of this item
    wall_breaker: 1         # will spawn exactly 1 of this item

# REQUIRED if `type` is 'procedural', has no effect on 'premade'
procedural_options:
    size: [11, 11]          # REQUIRED | size of the map [x, y]. Must be odd and > 5
    method: binarytree      # REQUIRED | method for procedural generation. 'binarytree'|'recursive'

# OPTIONAL. If defined will define how the level progresses to the next. If not, will just restart.
progression:
    next_level: testing2    # OPTIONAL | if defined will override all else and load the other map
    phase_multiply:         # OPTIONAL | will multiply the quantity defined for the next level phase
        procedural_size: 2  # OPTIONAL | multiplies the procedural map size by this factor
        enemies: 2          # OPTIONAL | multiplies the enemy count by this factor
        prey: 2             # OPTIONAL | multiplies the prey count by this factor
        items: 2            # OPTIONAL | multiplies the item count by this factor

```

If the level is of `type` `'premade'` then a map.txt is needed in the same directory.
