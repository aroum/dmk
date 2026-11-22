"""Exercise offline patching, model checks and the complete mocked WebHID stream."""
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys
import unittest
from unittest.mock import patch as mock_patch

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/hhkb"))
from patch_hybrid_hfb import assemble, bank_payload, patch, STOCK_TEMPLATES

VERSIONS = {"AHHX01": [10,4,2,9], "AHHS01": [10,8,0,3],
            "AHKS01": [10,9,0,4], "AHUX01": [10,0,4,8], "AHJX01": [10,2,4,9]}


def application(model):
    return struct.pack('<II', 0x20005000, 0x08010009) + b'\0' * 8 + f'DMKHHKB1|{model}|26.11.20-test|END'.encode()


def container(model, app=None, version=None):
    size = 282672 if model in ('AHUX01', 'AHJX01') else 65570
    tail = b'\xff' * (size - 65540 - 16) + model.encode() + bytes(version or VERSIONS[model]) + b'\xff' * 6
    return assemble(bank_payload(app or application(model)), tail)


class HtmlPatcherTest(unittest.TestCase):
    def run_js(self, body, **values):
        script = (ROOT / 'tools/hhkb/hhkb.html').read_text().split('<script>')[1].split('</script>')[0]
        harness = r'''
const vm=require('vm'),elements={};
function element(id) {
 if(!elements[id]) {
  const e={textContent:'',disabled:false,files:[],setAttribute(){}};
  Object.defineProperty(e,'onclick',{get(){return e.click},set(fn){e.click=fn;if(fn?.toString().includes('resolve()'))queueMicrotask(fn)}});
  elements[id]=e;
 }
 return elements[id];
}
const context=vm.createContext({TextDecoder,TextEncoder,Uint8Array,DataView,console,
 crypto:require('crypto').webcrypto,document:{getElementById:element},navigator:{},
 setTimeout:(fn,ms)=>{if(ms===10000)queueMicrotask(fn);return 0},clearTimeout(){}});
vm.runInContext(SCRIPT,context);
Object.assign(context,VALUES);
vm.runInContext('(async()=>{'+BODY+'})()',context).then(()=>console.log('OK')).catch(e=>{console.error(e);process.exitCode=1});
'''
        harness = harness.replace('SCRIPT', json.dumps(script)).replace('VALUES', json.dumps(values)).replace('BODY', json.dumps(body))
        return subprocess.run(['node'], input=harness, text=True, capture_output=True, check=True).stdout

    def test_browser_matches_python_and_preserves_radio(self):
        for model in ('AHUX01', 'AHJX01'):
            stock = container(model)
            app = application(model) + b'new application'
            tail = bytearray(stock[65540:])
            tail[-10:-6] = bytes([10, 0 if model == 'AHUX01' else 2, 5, 0])
            expected = assemble(bank_payload(app), bytes(tail))
            layout = 'ansi' if model == 'AHUX01' else 'jis'
            with mock_patch.dict(STOCK_TEMPLATES, {layout: ('test.hfb', hashlib.sha256(stock).hexdigest())}):
                self.assertEqual(patch(stock, app, layout=layout), expected)
            result = self.run_js('''
stockHashes[model]=hash;
const output=await patchFirmware(Uint8Array.from(stock),Uint8Array.from(app));
console.log(BufferNotAvailable=Array.from(output,x=>x.toString(16).padStart(2,'0')).join(''));
'''.replace('BufferNotAvailable=', ''), model=model, hash=hashlib.sha256(stock).hexdigest(), stock=list(stock), app=list(app))
            actual = bytes.fromhex(result.splitlines()[0])
            self.assertEqual(actual, expected)
            self.assertEqual(actual[65540:-10], stock[65540:-10])
            self.assertEqual(actual[-10:-6], bytes([10, 0 if model == 'AHUX01' else 2, 5, 0]))

    def test_patch_rejects_unsafe_inputs(self):
        stock = container('AHUX01')
        body = '''
async function rejects(fn){let rejected=false;try{await fn()}catch(e){rejected=true}if(!rejected)throw Error('accepted unsafe input')}
const original=Uint8Array.from(stock);
await rejects(()=>patchFirmware(original,Uint8Array.from(app))); // wrong official SHA
stockHashes.AHUX01=hash;
await rejects(()=>patchFirmware(original,Uint8Array.from(jis)));
await rejects(()=>patchFirmware(original,Uint8Array.from(legacy)));
await rejects(()=>patchFirmware(Uint8Array.from(classic),Uint8Array.from(app)));
stockHashes.AHUX01=otherHash;
await rejects(()=>patchFirmware(Uint8Array.from(other),Uint8Array.from(app))); // unsupported PFU version
const corrupt=original.slice();corrupt[80]^=1;
await rejects(()=>patchFirmware(corrupt,Uint8Array.from(app)));
'''
        other = container('AHUX01', version=[10,0,4,7])
        self.run_js(body, stock=list(stock), hash=hashlib.sha256(stock).hexdigest(),
                    app=list(application('AHUX01')), jis=list(application('AHJX01')),
                    legacy=list(struct.pack('<II',0x20005000,0x08010009)+bytes(40)),
                    classic=list(container('AHHX01')), other=list(other), otherHash=hashlib.sha256(other).hexdigest())

    def test_embedded_model_must_match_container(self):
        self.run_js('''
for(const file of files){let failed=false;try{validate(Uint8Array.from(file))}catch(e){failed=true}if(!failed)throw Error('accepted model mismatch')}
''', files=[list(container('AHUX01', application('AHJX01'))),
            list(container('AHHS01', application('AHHX01')))])

    def test_all_target_families_match_python_and_reject_cross_flashes(self):
        from .hhkb_model_cases import model_cases, MODELS
        self.run_js('''
for(const [number,expected] of Object.entries(cases)){
 if(deviceModel(number)!==expected)throw Error('wrong family: '+number);
 if(deviceModel('  '+number.toLowerCase()+'  ')!==expected)throw Error('normalization: '+number);
 // The actual PFU field contains at most 20 bytes.
 if(new TextEncoder().encode(number).length>20)continue;
 const info=new Uint8Array(64);info.set(new TextEncoder().encode(number),6);
 for(const model of expectedModels){
  let accepted=true;try{checkTarget(info,model)}catch(e){accepted=false}
  if(accepted!==(expected===model))throw Error('target validation: '+model+' / '+number);
 }
}
''', cases=model_cases(), expectedModels=list(MODELS))

    def test_original_flash_images_and_exact_device_variants(self):
        legacy = container('AHHX01', struct.pack('<II',0x20005000,0x08010009)+bytes(40))
        self.run_js('''
const data=Uint8Array.from(legacy);
let rejected=false;try{await verifyFlashImage(data)}catch(e){rejected=e.message.includes('legacy')}
if(!rejected)throw Error('unidentified image accepted');
officialHashes.AHHX01=hash;
if(await verifyFlashImage(data)!=='AHHX01')throw Error('recognized stock rejected');
const pairs=[['AHHX01','PD-KB401WSC'],['AHHS01','PD-KB401W'],['AHKS01','PD-KB401WSC'],['AHUX01','PD-KB820WS'],['AHJX01','PD-KB800BS']];
for(const [model,number] of pairs){const info=new Uint8Array(64);info.set(new TextEncoder().encode(number),6);let failed=false;try{checkTarget(info,model)}catch(e){failed=true}if(!failed)throw Error('variant mismatch accepted')}
let failed=false;try{validate(Uint8Array.from(wrongVersion))}catch(e){failed=true}if(!failed)throw Error('wrong version family accepted');
''', legacy=list(legacy),hash=hashlib.sha256(legacy).hexdigest(),
                    wrongVersion=list(container('AHHX01',version=[10,8,0,3])))

    def test_file_labels_and_disabled_mismatch(self):
        stock=container('AHUX01')
        self.run_js('''
stockHashes.AHUX01=hash;
function file(id,bytes){$(id).files=[{arrayBuffer:async()=>Uint8Array.from(bytes).buffer}]}
file('stock',stock);file('app',app);await inspectPatch();
if($('patchButton').disabled||!$('appInfo').textContent.includes('26.11.20-test')||!$('stockInfo').textContent.includes('A0.48'))throw Error('missing compatible file labels');
file('app',jis);await inspectPatch();
if(!$('patchButton').disabled||!$('patchStatus').textContent.includes('Incompatible'))throw Error('mismatch button enabled');
''', stock=list(stock),hash=hashlib.sha256(stock).hexdigest(),app=list(application('AHUX01')),jis=list(application('AHJX01')))

    def test_webhid_stream_and_recovery_identity_checks(self):
        for model, number in [('AHHX01','PD-KB401W'),('AHHS01','PD-KB401WSC'),
                              ('AHKS01','PD-KB421WSC'),('AHUX01','PD-KB800WS'),('AHJX01','PD-KB820BS')]:
            self.run_js(self.flash_harness(),image=list(container(model)),number=number,wrongAt=0)
        for wrong_at in (1,2):
            self.run_js(self.flash_harness(),image=list(container('AHUX01')),number='PD-KB800WS',wrongAt=wrong_at)

    def test_anniversary_model_suffix_before_and_after_recovery(self):
        for model, number, recovery in [
            ('AHHS01','PD-KB401WSC1','PD-KB401WSC'),
            ('AHKS01','PD-KB421BSC1','PD-KB421BSC'),
            ('AHUX01','PD-KB800YSC1','PD-KB800YSC'),
            ('AHJX01','PD-KB820WSC1','PD-KB820WSC')]:
            self.run_js(self.flash_harness(),image=list(container(model)),
                        number=number,recoveryNumber=recovery,wrongAt=0)

    def test_same_family_different_colour_is_not_the_same_recovery_target(self):
        self.run_js(self.flash_harness(),image=list(container('AHUX01')),
                    number='PD-KB800YSC1',recoveryNumber='PD-KB800BSC',
                    wrongAt=0,rejectRecovery=True)

    @staticmethod
    def flash_harness():
        return '''
const packets=[];let infoCount=0;
const device={opened:false,collections:[{usagePage:0xff00,usage:1,inputReports:[{reportId:0}],outputReports:[{reportId:0}]}],
open:async()=>{device.opened=true},close:async()=>{device.opened=false},
addEventListener:(_,fn)=>{device.callback=fn},removeEventListener(){},
sendReport:async(_,packet)=>{packets.push(packet);const reply=new Uint8Array(64);reply.set([85,85,packet[2],0,0,0]);
if(packet[2]===2){infoCount++;reply.set(new TextEncoder().encode(infoCount===wrongAt?'PD-KB401W':(infoCount===2 && typeof recoveryNumber!=='undefined'?recoveryNumber:number)),6)}
device.callback({reportId:0,data:new DataView(reply.buffer)});
}};
navigator.hid={requestDevice:async()=>[device]};
let failed=false;try{await flash(Uint8Array.from(image))}catch(e){failed=true}
if(wrongAt || (typeof rejectRecovery!=='undefined' && rejectRecovery)){if(!failed||packets.some(p=>p[2]===225))throw Error('wrote to wrong target');if(wrongAt===1&&packets.some(p=>p[2]===224))throw Error('rebooted wrong target')}
else {if(failed)throw Error('flash failed');const body=[];for(const p of packets.filter(p=>p[2]===226))body.push(...p.slice(7,7+p[4]-2));if(JSON.stringify(body)!==JSON.stringify(image.slice(2)))throw Error('payload mismatch');if(packets.at(-1)[2]!==227||infoCount!==2)throw Error('missing E3 or recovery check')}
if(device.opened)throw Error('device leaked');
'''


if __name__ == '__main__':
    unittest.main()
