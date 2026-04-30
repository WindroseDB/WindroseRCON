import socket
import struct
from cryptography.hazmat.primitives.ciphers.aead import AESGCM
import os

RCON_HOST = "127.0.0.1"
RCON_PORT = 27065
RCON_PASSWORD = "windrose_admin"
AES_KEY = ""  # Put your AES key here from the settings.ini file

class SecureRCONClient:
    SERVERDATA_AUTH = 3
    SERVERDATA_AUTH_RESPONSE = 2
    SERVERDATA_EXECCOMMAND = 2
    SERVERDATA_RESPONSE_VALUE = 0

    def __init__(self, host, port, password, aes_key_hex):
        self.host = host
        self.port = port
        self.password = password
        self.sock = None
        self.request_id = 0
        
        if aes_key_hex:
            self.aes_key = bytes.fromhex(aes_key_hex)
            self.aesgcm = AESGCM(self.aes_key)
            self.encrypted = True
        else:
            self.encrypted = False

    def connect(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.host, self.port))
        return self.authenticate()

    def disconnect(self):
        if self.sock:
            self.sock.close()
            self.sock = None

    def send_packet(self, packet_type, body):
        self.request_id += 1
        body_bytes = body.encode('utf-8') + b'\x00'
        packet_size = len(body_bytes) + 10
        
        packet = struct.pack('<i', packet_size)
        packet += struct.pack('<i', self.request_id)
        packet += struct.pack('<i', packet_type)
        packet += body_bytes
        packet += b'\x00'
        
        if self.encrypted:
            nonce = os.urandom(12)
            ciphertext = self.aesgcm.encrypt(nonce, packet, None)
            encrypted_packet = nonce + ciphertext
            
            encrypted_size = struct.pack('<I', len(encrypted_packet))
            self.sock.sendall(encrypted_size + encrypted_packet)
        else:
            self.sock.sendall(packet)
        
        return self.request_id

    def receive_packet(self):
        if self.encrypted:
            size_data = self.sock.recv(4)
            if len(size_data) < 4:
                return None
            
            encrypted_size = struct.unpack('<I', size_data)[0]
            encrypted_data = b''
            while len(encrypted_data) < encrypted_size:
                chunk = self.sock.recv(encrypted_size - len(encrypted_data))
                if not chunk:
                    return None
                encrypted_data += chunk
            
            nonce = encrypted_data[:12]
            ciphertext = encrypted_data[12:]
            
            try:
                packet = self.aesgcm.decrypt(nonce, ciphertext, None)
            except Exception as e:
                print(f"Decryption failed: {e}")
                return None
            
            size = struct.unpack('<i', packet[0:4])[0]
            req_id = struct.unpack('<i', packet[4:8])[0]
            pkt_type = struct.unpack('<i', packet[8:12])[0]
            body = packet[12:-2].decode('utf-8', errors='ignore')
        else:
            size_data = self.sock.recv(4)
            if len(size_data) < 4:
                return None
            
            size = struct.unpack('<i', size_data)[0]
            data = b''
            while len(data) < size:
                chunk = self.sock.recv(size - len(data))
                if not chunk:
                    return None
                data += chunk
            
            req_id = struct.unpack('<i', data[0:4])[0]
            pkt_type = struct.unpack('<i', data[4:8])[0]
            body = data[8:-2].decode('utf-8', errors='ignore')
        
        return {
            'id': req_id,
            'type': pkt_type,
            'body': body
        }

    def authenticate(self):
        self.send_packet(self.SERVERDATA_AUTH, self.password)
        response = self.receive_packet()
        
        if response and response['id'] != -1:
            print("✓ Authentication successful")
            return True
        else:
            print("✗ Authentication failed")
            return False

    def execute_command(self, command):
        self.send_packet(self.SERVERDATA_EXECCOMMAND, command)
        response = self.receive_packet()
        
        if response:
            return response['body']
        return None

def main():
    print("=" * 50)
    print("Windrose Secure RCON Client (AES-256-GCM)")
    print("=" * 50)
    
    if not AES_KEY:
        print("ERROR: Set AES_KEY variable with your 64-char hex key from settings.ini")
        return
    
    print(f"Connecting to {RCON_HOST}:{RCON_PORT}...")
    print("Encryption: ENABLED")
    
    try:
        client = SecureRCONClient(RCON_HOST, RCON_PORT, RCON_PASSWORD, AES_KEY)
        client.connect()
        
        print("\nRCON Console Ready. Type 'quit' to exit.\n")
        
        while True:
            try:
                command = input("RCON> ")
                if command.lower() in ['quit', 'exit']:
                    break
                
                if command.strip():
                    response = client.execute_command(command)
                    if response:
                        print(f"Response: {response}")
                    else:
                        print("No response received")
            except KeyboardInterrupt:
                break
        
        client.disconnect()
        print("\nDisconnected.")
        
    except ConnectionRefusedError:
        print("Connection refused. Make sure the server is running.")
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()
