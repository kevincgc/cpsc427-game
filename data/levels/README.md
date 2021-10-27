Each level should be a folder.
There should be a `level.yml` file inside, following the format:

```yaml
name: Testing Map 1         # REQUIRED | this is a human-readable name
type: premade               # REQUIRED | either 'premade'|'procedural'

# REQUIRED if `type` is 'procedural', has no effect on 'premade'
procedural_options:
    size: [10, 10]          # REQUIRED | size of the map [x, y]
```
