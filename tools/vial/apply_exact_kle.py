import json
import re
import sys

raw = """[
[{y:0.13,x:3,c:"#f7f2ea"},"",{x:8.25},""],
[{y:-0.75,x:2},"",{x:1},"",{x:6.25},"",{x:1},""],
[{y:-0.88,x:5},"",{x:4.25},""],
[{y:-0.75,x:1},"",{x:12.25},""],
[{y:-0.87,c:"#909596"},"",{x:14.25},""],
[{y:-0.75,x:3,c:"#f7f2ea"},"",{x:8.25},""],
[{y:-0.75,x:2},"",{x:1,n:true},"",{x:6.25,n:true},"",{x:1},""],
[{y:-0.88,x:5},"",{x:4.25},""],
[{y:-0.75,x:1},"",{x:12.25},""],
[{y:-0.87,c:"#909596"},"",{x:14.25},""],
[{y:-0.75,x:3,c:"#f7f2ea"},"",{x:8.25},""],
[{y:-0.75,x:2},"",{x:1},"",{x:6.25},"",{x:1},""],
[{y:-0.88,x:5},"",{x:4.25},""],
[{y:-0.75,x:1},"",{x:12.25},""],
[{y:-0.87,c:"#909596"},"",{x:14.25},""],
[{y:-0.5,x:4},"",{x:6.25},""],
[{y:-0.88,x:5,n:true},"",{x:4.25,n:true},""],
[{y:-0.87,x:6},"",{x:2.25},""]
]"""

labels = [
    # Top row
    "0,3", "3,2",
    "0,2", "0,4", "3,1", "3,3",
    "0,5", "3,0",
    "0,1", "3,4",
    "0,0", "3,5",
    # Middle row
    "1,3", "4,2",
    "1,2", "1,4", "4,1", "4,3",
    "1,5", "4,0",
    "1,1", "4,4",
    "1,0", "4,5",
    # Bottom row
    "2,3", "5,2",
    "2,2", "2,4", "5,1", "5,3",
    "2,5", "5,0",
    "2,1", "5,4",
    "2,0", "5,5",
    # Thumbs
    "6,3", "6,2",
    "6,4", "6,1",
    "6,5", "6,0"
]

count = 0
def repl(m):
    global count
    lbl = labels[count]
    count += 1
    return f'"{lbl}"'

result = re.sub(r'""', repl, raw)
res_json = re.sub(r'([a-zA-Z0-9_]+):', r'"\1":', result)
parsed_keymap = json.loads(res_json)

vial_data = {
    "name": "Kabarga DMK",
    "vendorId": "0xCafe",
    "productId": "0x4010",
    "lighting": "qmk_rgblight",
    "matrix": {
        "rows": 7,
        "cols": 6
    },
    "layouts": {
        "keymap": parsed_keymap
    }
}

with open("keyboards/kabarga/vial.json", "w") as f:
    json.dump(vial_data, f, indent=2)

print("Generated vial.json without a:7 successfully!")
