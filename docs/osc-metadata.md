# OSC presentation metadata

These additive, read-only notifications use Caster's existing OSC listener,
configured outbound destination and state channel. Existing control commands and
state messages are unchanged. Clients can ignore these additional addresses.

| Address | OSC type tags | Arguments |
| --- | --- | --- |
| `/cbp/<player-id>/m/color` | `,s` | One uppercase `#RRGGBB` string |
| `/cbp/<player-id>/m/grid` | `,ii` | Two signed int32 values: **row, column** |

Coordinates are zero-based, with `(0,0)` at the top left. They come from the
actual Qt grid layout, not sorted player IDs or UDP arrival order. The current
board has 4 rows and 8 columns: `1` is `(0,0)`, `8` is `(0,7)`, `Z` is `(3,0)`
and `,` is `(3,7)`. All slots, including empty ones, have both messages. Slot
positions are currently immutable; resizing and scrolling do not change them.

Colour represents the **top stop of the button's background gradient**, as
interpreted by Qt from its applied stylesheet. It does not include the temporary
playback/progress overlay, icons or text. The default grey is `#DADADA`. A picker
colour uses the generated gradient's top stop, including its HSL *adjustments*
to the base gradient; the sliders are not an absolute HSL colour. Reading Qt's
brush preserves its interpretation of fractional RGB components and old saved
stylesheets. Missing/unsupported gradient styling falls back to `#DADADA`.
No additional colour or position state is persisted.

A normal `/glo/sync` request (existing float argument `1.0`) includes both
messages for every slot on the active board, alongside existing state. Existing
syncs when starting OSC and switching tabs also include them. Profile loads and
reloads use the applied loaded appearance on the next sync, including a return
to default grey. After accepting a colour edit, a changed representative colour
is published immediately for the active board. Preview movements, Cancel,
Escape, window dismissal and acceptance of an unchanged colour do not publish
colour changes. Inactive-board edits are not forwarded. The existing transport
sends nothing while OSC is disabled; UDP does not require a connected peer.

Example messages (address, type tags, arguments):

```text
/glo/sync          ,f   1.0
/cbp/1/m/color     ,s   "#DADADA"
/cbp/1/m/grid      ,ii  0 0
/cbp/,/m/color     ,s   "#DADADA"
/cbp/,/m/grid      ,ii  3 7
```

The metadata addresses are notifications, not writable commands. Clients should
key state by player ID and use explicit coordinates to place buttons. Packet
ordering and delivery are not guaranteed by UDP; request another sync if needed.
