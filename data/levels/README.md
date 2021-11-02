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
```

If the level is of `type` `'premade'` then a map.txt is needed in the same directory.
