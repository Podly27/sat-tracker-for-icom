"""Exercise the actual C HTTP/TLE parser and mocked ICAP transport."""
import ctypes as C
from pathlib import Path
import shutil, subprocess, sys, tempfile, unittest
ROOT=Path(__file__).resolve().parents[1]
@unittest.skipUnless(shutil.which('cc'), 'requires host C compiler')
class NetworkParserTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp=tempfile.TemporaryDirectory();lib=Path(cls.tmp.name)/'net.so'
        subprocess.run(['cc','-std=c11','-O1','-dynamiclib' if sys.platform=='darwin' else '-shared','-fPIC',str(ROOT/'src/tle_network.c'),'-o',str(lib)],check=True)
        cls.lib=C.CDLL(str(lib));cls.lib.tle_http_init.argtypes=[C.c_void_p]
        cls.lib.tle_http_feed.argtypes=[C.c_void_p,C.c_char_p,C.c_uint]
        cls.lib.tle_http_finish.argtypes=[C.c_void_p]
        cls.tle=(ROOT/'tests/fixtures/iss-historical.tle').read_bytes().replace(b'\r\n',b'\n')
    @classmethod
    def tearDownClass(cls):cls.tmp.cleanup()
    def parse(self,wire,block=1):
        state=C.create_string_buffer(4096);self.lib.tle_http_init(state)
        for i in range(0,len(wire),block):
            piece=wire[i:i+block]
            if self.lib.tle_http_feed(state,piece,len(piece)):return -1
        return self.lib.tle_http_finish(state)
    def response(self,body=None,headers=b'',status=b'200 OK'):
        if body is None:body=self.tle
        return b'HTTP/1.1 '+status+b'\r\n'+headers+b'\r\n'+body
    def test_content_length_fragmentation(self):
        for block in [1,2,7,64,256,4096]:
            with self.subTest(block=block):
                self.assertEqual(self.parse(self.response(headers=b'Content-Length: '+str(len(self.tle)).encode()+b'\r\n'),block),0)
    def test_close_delimited(self):self.assertEqual(self.parse(self.response()),0)
    def test_no_final_newline(self):self.assertEqual(self.parse(self.response(self.tle.rstrip(b'\n'))),0)
    def test_chunked(self):
        for block in [1,17,256]:
            body=b''.join(hex(len(self.tle[i:i+11]))[2:].encode()+b';x=y\r\n'+self.tle[i:i+11]+b'\r\n'for i in range(0,len(self.tle),11))+b'0\r\n\r\n'
            self.assertEqual(self.parse(self.response(body,b'Transfer-Encoding: chunked\r\n'),block),0)
    def test_http_errors_and_redirects(self):
        for status in [b'301 Moved',b'403 Forbidden',b'500 Error']:
            self.assertNotEqual(self.parse(self.response(status=status)),0)
    def test_bad_checksum(self):
        body=self.tle.replace(b'26323.84989826',b'26323.84989827');self.assertNotEqual(self.parse(self.response(body)),0)
    def test_duplicate_iss(self):self.assertNotEqual(self.parse(self.response(self.tle+self.tle)),0)
    def test_non_iss(self):self.assertNotEqual(self.parse(self.response(self.tle.replace(b'25544',b'00000'))),0)
    def test_html(self):self.assertNotEqual(self.parse(self.response(b'<html>Unavailable</html>')),0)
    def test_oversized_line_and_header(self):
        self.assertNotEqual(self.parse(self.response(b'x'*160)),0)
        self.assertNotEqual(self.parse(self.response(headers=b'X-Test: '+b'x'*1024+b'\r\n')),0)
    def test_truncated_and_excess_body(self):
        for n in [len(self.tle)-1,len(self.tle)+1]:
            self.assertNotEqual(self.parse(self.response(headers=b'Content-Length: '+str(n).encode()+b'\r\n')),0)
    def test_ambiguous_headers(self):
        for headers in [b'Content-Length: 1\r\nContent-Length: 1\r\n',b'Transfer-Encoding: chunked\r\nContent-Length: 10\r\n',b'Content-Encoding: gzip\r\n',b'Content-Length: -1\r\n',b'Transfer-Encoding: gzip\r\n']:
            self.assertNotEqual(self.parse(self.response(headers=headers)),0)
    def test_bad_chunk_framing(self):
        for body in [b'fffffff\r\n',b'xx\r\n',b'0\n\n',b'0\r\nX: trailer\r\n\r\n',b'1\r\nxBAD']:
            self.assertNotEqual(self.parse(self.response(body,b'Transfer-Encoding: chunked\r\n')),0)
    def test_transport_cleanup(self):
        binary=Path(self.tmp.name)/'transport'
        subprocess.run(['cc','-std=c11','-I'+str(ROOT/'src'),str(ROOT/'tests/network_transport_harness.c'),str(ROOT/'src/tle_network.c'),'-o',str(binary)],check=True)
        for case in ['success','offline','connect-fail','send-fail','read-fail','cancel','fallback','timeout','unsupported','profiles','numeric']:
            with self.subTest(case=case):subprocess.run([str(binary),case,str(ROOT/'tests/fixtures/iss-historical.tle')],check=True)
