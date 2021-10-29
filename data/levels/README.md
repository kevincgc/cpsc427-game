Each level should be a folder.
There should be a `level.yml` file inside, following the format:

```yaml
name: Testing Map 1         # REQUIRED | this is a human-readable name
type: premade               # REQUIRED | either 'premade'|'procedural'

# OPTIONAL | every enemy is optional, as well as the 'enemies' property. If no enemies
# are specified then no enemies will spawn.
enemies:
    spikes: 2
    drones: 3

# REQUIRED if `type` is 'procedural', has no effect on 'premade'
procedural_options:
    size: [11, 11]          # REQUIRED | size of the map [x, y]. Must be odd and > 5
    method: binarytree      # REQUIRED | method for procedural generation. 'binarytree'|'recursive'
```
