import importlib.util
from pathlib import Path
import struct,tempfile,unittest
ROOT=Path(__file__).resolve().parents[1]
def load(name):
    spec=importlib.util.spec_from_file_location(name,ROOT/'tools'/f'{name}.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
convert=load('bin_to_uf2').convert
audit=load('audit').audit
class PackagingTests(unittest.TestCase):
    def fixture(self,folder):
        binary=folder/'bitdoglab_arcade.bin';uf2=folder/'bitdoglab_arcade.uf2';binary.write_bytes(bytes(range(256))*3+b'abc'*19);convert(binary,uf2);return binary,uf2
    def test_roundtrip_and_padding_cover_every_binary_byte(self):
        with tempfile.TemporaryDirectory() as d:
            folder=Path(d);self.fixture(folder);report=audit(folder);self.assertEqual(report['blocks'],4);self.assertEqual(report['games'],8)
    def test_wrong_family_or_changed_payload_is_rejected(self):
        for mode in ['family','payload','padding']:
            with self.subTest(mode=mode),tempfile.TemporaryDirectory() as d:
                folder=Path(d);_,uf2=self.fixture(folder);b=bytearray(uf2.read_bytes())
                if mode=='family':struct.pack_into('<I',b,28,0)
                elif mode=='payload':b[32]^=1
                else:b[3*512+32+57]=1
                uf2.write_bytes(b)
                with self.assertRaises(ValueError):audit(folder)
    def test_duplicate_or_missing_blocks_are_rejected(self):
        for mode in ['duplicate','missing']:
            with self.subTest(mode=mode),tempfile.TemporaryDirectory() as d:
                folder=Path(d);_,uf2=self.fixture(folder);b=bytearray(uf2.read_bytes())
                if mode=='duplicate':struct.pack_into('<I',b,512+20,0)
                else:b=b[:-512]
                uf2.write_bytes(b)
                with self.assertRaises(ValueError):audit(folder)
    def test_converter_requires_explicit_replace(self):
        with tempfile.TemporaryDirectory() as d:
            folder=Path(d);binary,uf2=self.fixture(folder)
            with self.assertRaises(FileExistsError):convert(binary,uf2)
            convert(binary,uf2,replace=True);audit(folder)
if __name__=='__main__':unittest.main()
