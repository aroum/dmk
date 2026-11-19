"""Compare the standalone browser patcher against the Python HFB packer."""
import json
from pathlib import Path
import struct
import subprocess
import sys
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/hhkb"))
from patch_hybrid_hfb import assemble, bank_payload


class HtmlPatcherTest(unittest.TestCase):
    def test_webhid_packet_stream(self):
        from patch_hybrid_hfb import make_stm32_only
        image = make_stm32_only(struct.pack("<II", 0x20005000, 0x08010009) + bytes(4), layout="ansi")
        script = (ROOT / "tools/hhkb/hhkb.html").read_text().split("<script>")[1].split("</script>")[0]
        harness = """
const vm=require('vm'),packets=[],elements={};
const device={opened:false,collections:[{usagePage:0xff00,usage:1,inputReports:[{reportId:0}],outputReports:[{reportId:0}]}],
open:async()=>{device.opened=true},close:async()=>{device.opened=false},
addEventListener:(_,fn)=>{device.callback=fn},removeEventListener:()=>{},
sendReport:async(_,packet)=>{packets.push(packet);
 const reply=new Uint8Array(64);reply.set([85,85,packet[2],0,0,0]);
 if(packet[2]===2)reply.set(new TextEncoder().encode('PD-KB800WS'),6);
 device.callback({reportId:0,data:new DataView(reply.buffer)});
}};
const context=vm.createContext({TextDecoder,Uint8Array,DataView,console,
navigator:{hid:{requestDevice:async()=>[device]}},
document:{getElementById:id=>elements[id]??=(id==='flashButton'?{set onclick(fn){if(fn && fn.toString().includes("resolve()"))queueMicrotask(fn)}}:{textContent:''})},
setTimeout:(fn,ms)=>{if(ms===10000)queueMicrotask(fn);return 0},clearTimeout:()=>{}});
vm.runInContext(SCRIPT,context);context.image=Uint8Array.from(IMAGE);
vm.runInContext('flash(image)',context).then(()=>{
 const e2=packets.filter(p=>p[2]===226),body=[];
 for(const packet of e2)body.push(...packet.slice(7,7+packet[4]-2));
 if(!Buffer.from(body).equals(Buffer.from(context.image.slice(2))))throw Error('payload mismatch');
 if(packets.at(-1)[2]!==227)throw Error('missing E3');
 console.log('complete stream verified');
}).catch(e=>{console.error(e);process.exitCode=1});
"""
        harness = harness.replace("SCRIPT", json.dumps(script)).replace("IMAGE", json.dumps(list(image)))
        result = subprocess.run(["node"], input=harness, text=True, capture_output=True, check=True)
        self.assertIn("complete stream verified", result.stdout)

    def test_browser_matches_python_and_preserves_radio(self):
        application = struct.pack("<II", 0x20005000, 0x08010009) + bytes(range(256))
        for model, size in ((b"AHHX01", 65570), (b"AHHS01", 65570),
                            (b"AHKS01", 65570), (b"AHUX01", 282672),
                            (b"AHJX01", 282672)):
            tail = bytes([0xA5]) * (size - 65540 - 16) + model + bytes([10, 0, 4, 8]) + bytes([255]) * 6
            stock = assemble(bank_payload(application), tail)
            replacement = application + b"changed"
            expected = assemble(bank_payload(replacement), tail)
            script = (ROOT / "tools/hhkb/hhkb.html").read_text().split("<script>")[1].split("</script>")[0]
            harness = """
const vm=require('vm');
const document={getElementById:()=>({})};
const context=vm.createContext({document,TextDecoder,Uint8Array,DataView,console});
vm.runInContext(SCRIPT,context);
context.stock=Uint8Array.from(STOCK);
context.app=Uint8Array.from(APP);
process.stdout.write(Buffer.from(vm.runInContext('patchFirmware(stock,app)',context)).toString('hex'));
"""
            harness = harness.replace("SCRIPT", json.dumps(script)).replace("STOCK", json.dumps(list(stock))).replace("APP", json.dumps(list(replacement)))
            result = subprocess.run(["node"], input=harness, text=True, capture_output=True, check=True)
            self.assertEqual(bytes.fromhex(result.stdout), expected)


if __name__ == "__main__":
    unittest.main()
