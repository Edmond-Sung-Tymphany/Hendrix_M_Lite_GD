# Proto files

Proto files - source files used by google protocol buffers compiler

## Definition of the core submessages

* **Request** and **Response**: those two submessages have the same 2 common fields:
    * id - identification of message that can be used by higher-level logic for pairing request and response (optional)
    * ReqResp request/response - it is an enum type field, that identifies the transfer.
        Those two fields should be always be copied from the request to response so
        that the higher level logic is able to pair request and response.
* **Event** notify about sender's state. May contain some data attached to the event.

Receiver of the request may respond by the means of a generic response (now defined in common.proto) - e.g. `NOT_SUPPORTED`.
Messages other than those 3 will be embedded in the {FepAse/AseFep}.{Req/Resp} fields.
*NO* other messages should be added into toplevel AseFepMessage/FepAseMessage.

## Compiling .proto files for **nanopb**

[NanoPb documentation](https://koti.kapsi.fi/jpa/nanopb/docs/index.html)  

Binary releases of nanopb are available at this [link](https://koti.kapsi.fi/jpa/nanopb/download/).  
In the official [README.md](https://github.com/nanopb/nanopb/blob/master/README.md#using-the-protocol-buffers-compiler-protoc),
there are steps on how to compile .proto files for nanopb.

### Example
Lets have all .proto files located in `/ase-fep-proto/definitions` and we want to put
compiled files into directory `/nanopb-compiled-defs`.  
Lets say we are using nanopb version 0.3.6 that is downloaded in `/nanopb-0.3.6`.

#### Step 1

Run *make* command in `/nanopb-0.3.6/generator/proto`.
#### Step 2

```
protoc --plugin=protoc-gen-nanopb="/nanopb-0.3.6/generator/protoc-gen-nanopb"\
			--nanopb_out="${PROTO_FILES_COMPILED_PREFIX}"\
			-I${PROTO_PREFIX}\
			${PROTO_FILES}'
```
`${PROTO_PREFIX}` a directory in which the proto files are stored (substitute with `/ase-fep-proto/definitions`).  
Substitute `${PROTO_FILES}` with all files located in the _definitions_ directory
(`${PROTO_PREFIX}/ase-fep.proto ${PROTO_PREFIX}/ase-fep-ReqResp.proto ...`)
Substitute `${PROTO_FILES_COMPILED_PREFIX}` with a directory to which the definitions will be put
(`/nanopb-compiled-defs`)



