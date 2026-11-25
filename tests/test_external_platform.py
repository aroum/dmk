"""External platform discovery and driver replacement, including cloned config repos."""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]

class ExternalPlatformTest(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory(prefix='dmk external [port] ')
        self.addCleanup(self.temp.cleanup)
        self.base=Path(self.temp.name)
        self.repo=self.base/'source'
        shutil.copytree(ROOT/'tests/platform_repo_example',self.repo)
        self.run_cmd(['git','init','-q',str(self.repo)],check=True)
        self.run_cmd(['git','-C',str(self.repo),'add','.'],check=True)
        self.run_cmd(['git','-C',str(self.repo),'-c','user.name=DMK Tests','-c','user.email=tests@example.invalid','commit','-qm','fixture'],check=True)
        self.clone=self.base/'cloned configs'
        self.run_cmd(['git','clone','-q',str(self.repo),str(self.clone)],check=True)
        self.keyboard=self.clone/'keyboards/demo'
        self.platform=self.clone/'platforms/external_demo'

    def run_cmd(self,args,**kwargs):
        env=os.environ.copy()
        env.update(GIT_CONFIG_GLOBAL=os.devnull,GIT_CONFIG_NOSYSTEM='1')
        return subprocess.run(args,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,env=env,**kwargs)

    def resolve(self,explicit=None):
        script=self.base/'resolve.cmake'
        script.write_text(f'''cmake_minimum_required(VERSION 3.20)
set(DMK_ROOT "{ROOT}")
set(ACTUAL_KBD_DIR "{self.keyboard}")
set(MCU external_demo)
include("{ROOT}/cmake/dmk_platform.cmake")
file(WRITE "{self.base}/resolved" "${{PLATFORM_DIR}}")
''')
        args=['cmake']
        if explicit is not None:args.append(f'-DDMK_PLATFORM_DIR={explicit}')
        return self.run_cmd(args+['-P',str(script)])

    def test_config_relative_path_from_clone(self):
        result=self.resolve();self.assertEqual(result.returncode,0,result.stdout)
        self.assertEqual(Path((self.base/'resolved').read_text()).resolve(),self.platform.resolve())

    def test_repo_autodiscovery_and_commented_hint(self):
        config=self.keyboard/'config.h'
        config.write_text(config.read_text().replace('#define DMK_PLATFORM_DIR', '// #define DMK_PLATFORM_DIR'))
        result=self.resolve();self.assertEqual(result.returncode,0,result.stdout)
        self.assertEqual(Path((self.base/'resolved').read_text()).resolve(),self.platform.resolve())

    def test_explicit_path_wins_and_missing_manifest_fails(self):
        other=self.base/'other platform';other.mkdir();(other/'platform.cmake').write_text('# port')
        result=self.resolve(other);self.assertEqual(result.returncode,0,result.stdout)
        self.assertEqual((self.base/'resolved').read_text(),str(other))
        result=self.resolve(self.base/'missing')
        self.assertNotEqual(result.returncode,0);self.assertIn('requires platform.cmake',result.stdout)

    def test_driver_replacement_and_exclusion(self):
        script=self.base/'drivers.cmake'
        (self.platform/'drivers/gpio.c').write_text('// replacement')
        (self.platform/'drivers/app_usb_tinyusb.c').write_text('// transport replacement')
        script.write_text(f'''cmake_minimum_required(VERSION 3.20)
set(DMK_ROOT "{ROOT}")
set(ACTUAL_KBD_DIR "{self.keyboard}")
set(MCU external_demo)
include("{ROOT}/cmake/dmk_platform.cmake")
set(DMK_PLATFORM_DRIVER_DIR "{self.platform}/drivers")
set(DMK_PLATFORM_EXCLUDE_DRIVERS adc.c)
dmk_platform_resolve_sources(result "{ROOT}/dmk_core/drivers/gpio.c"
 "{ROOT}/dmk_core/drivers/adc.c" "{ROOT}/dmk_core/features/layers.c"
 "{ROOT}/dmk_core/drivers/app_usb_tinyusb.c" "{ROOT}/dmk_core/drivers/gpio.c")
file(WRITE "{self.base}/drivers" "${{result}}")
''')
        result=self.run_cmd(['cmake','-P',str(script)])
        self.assertEqual(result.returncode,0,result.stdout)
        self.assertEqual((self.base/'drivers').read_text().split(';'),[
            str(self.platform/'drivers/gpio.c'),str(ROOT/'dmk_core/features/layers.c'),
            str(self.platform/'drivers/app_usb_tinyusb.c')])

    @unittest.skipUnless(os.environ.get('DMK_TEST_EXTERNAL_ARM')=='1','enable DMK_TEST_EXTERNAL_ARM for real ARM firmware build')
    def test_cloned_repo_firmware_build(self):
        # Exercise the user-facing wrapper, early MCU selection, replacement HAL,
        # feature-aware platform callback, regular module discovery and artifacts.
        out=self.base/'firmware'
        result=self.run_cmd(['bash',str(ROOT/'build_all.sh'),'-b',str(self.keyboard),
            '--build-dir',str(out)],cwd=ROOT)
        self.assertEqual(result.returncode,0,result.stdout[-12000:])
        self.assertTrue((out/'dmk_demo_external_demo.bin').is_file())
        self.assertTrue((out/'dmk_demo_external_demo.hex').is_file())
        cache=(out/'CMakeCache.txt').read_text()
        self.assertIn('MCU:STRING=external_demo',cache)
        commands=(out/'CMakeFiles/dmk_demo_external_demo.dir/build.make').read_text()
        self.assertIn('external-drivers/gpio.c',commands)
        self.assertIn('modules/probe/probe.c',commands)

        # A repo-root selection must detect the MCU before toolchain selection.
        config=self.keyboard/'config.h'
        config.write_text(config.read_text().replace('#define DMK_PLATFORM_DIR','// #define DMK_PLATFORM_DIR'))
        repo_build=self.base/'root-build'
        result=self.run_cmd(['cmake','-S',str(ROOT),'-B',str(repo_build),
            f'-DKEYBOARD_DIR={self.clone}'],cwd=ROOT)
        self.assertEqual(result.returncode,0,result.stdout)
        result=self.run_cmd(['cmake','--build',str(repo_build),'--parallel','2'])
        self.assertEqual(result.returncode,0,result.stdout[-12000:])
        self.assertTrue((repo_build/'dmk_demo_external_demo.bin').is_file())
        # CLI override must take precedence over a broken config hint.
        config.write_text(config.read_text().replace('// #define DMK_PLATFORM_DIR "../../platforms/external_demo"',
            '#define DMK_PLATFORM_DIR "../../missing"'))
        result=self.run_cmd(['bash',str(ROOT/'build_all.sh'),'-b',str(self.keyboard),
            '--platform-dir',str(self.platform),'--build-dir',str(out)],cwd=ROOT)
        self.assertEqual(result.returncode,0,result.stdout[-12000:])
