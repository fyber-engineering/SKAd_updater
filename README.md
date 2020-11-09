# SKAd Updater 

SKAdNetwork ID auto updater

## Install

##### Using Homebrew:
    brew tap fyber-engineering/skad_updater
    brew install skad_updater

## Use

### Synopsis

    skad_updater ( (--help | -h) | (--show_networks) | --plist_file_path plist-file-path (--network_list <comma-separated-network-names> | --pod_file_path <pod-file-path>) [--dry_run] )

### Description
 Pull the most up-to-date SKAdNetworks from https://github.com/fyber-engineering/SKAdNetworks and updates the info.plist appropriately.

 The list of required networks can be requested in several ways:

1.   Explicitly:
     1.  Asking for a list of supported ad network names with the `--show_networks` flag.
     1.  Passing the `[--network_list <network-name-list>]` parameter where <network-name-list> is a comma separated list of network names.
1.   Automatically deriving the required networks from a `pod file`, by using the `[ --pod_file_path <pod-file-path> ]` parameter where `<pod-file-path>` is the path to the pod file.
1. Combining the automatically derived networks from the `pod file` and an explicit network list, by using both the `[ --pod_file_path <pod-file-path> ]` and the `[--network_list <network-name-list>]` parameters.

#### Parameters:

| Parameter | Value  | Description  |
| :- | :-: | :-: |
| `--plist_file_path` | \<plist-file-path\> | The plist file path. |
| `--network_list` | \<comma-separated-network-names\> | Request for a specific list of networks to update. The argument is a comma separated list of network names. |
| `--pod_file_path` | \<pod-file-path\> | Update all the networks found in the pod file.  The argument is the path to the pod file. |
| **Optional Parameters** ||
| `--dry_run` | | Perform a dry-run. Prints out the new `plist` file instead of overwriting.|
| `--show_networks` | | Show the list of supported network names.| 
| `--help, -h` | | Give a help message and exit. |

#### Examples
     skad_updater --help 
     
     skad_updater --show_networks
     
     skad_updater --plist_file_path <Path to plist> --pod_file_path <Path to Pod File> --dry_run

     skad_updater --plist_file_path <Path to plist> --pod_file_path <Path to Pod File>

     skad_updater --plist_file_path <Path to plist> --network_list <CSV network list> --dry_run

     skad_updater --plist_file_path <Path to plist> --network_list <CSV network list>

     skad_updater --plist_file_path <Path to plist> --network_list <CSV network list> --pod_file_path <Path to Pod File> --dry_run

     skad_updater --plist_file_path <Path to plist> --network_list <CSV network list> --pod_file_path <Path to Pod File>

### Backups
Current/Previous info.plist will be backed up to info.plist.bak.X in the same directory in case the plist is modified, where X is the number of backup.

### Reformatting the plist file
After new networks were added, the plist file will be formatted according to the standard XML indentation formatting. The xcode PList indentaion formatting might differ from the standard XML's.

*Don't worry, it works the same.*

Making a modification to the plist inside xcode will reformat it back to the xcode standard, Which might result in weird pull-request diffs.

If this is an issue for you, you can run this after the skad_updater to reformat according to the xcode standard:
```
plutil -convert xml1 <path_to_plist_file>
```
### Debugging

#### Debug logs
* Enable
```
export FYBER_SKAD_DEBUG_LOG=''
```
* Disable
```
unset FYBER_SKAD_DEBUG_LOG
```

#### Mock service
##### Background
The skad_updater depends on the most up-to-date information about the list of SKAdNetworks. 
To achieve this, the skad_updater recieves the latest infromation from a backend web-service.

In order to run the tests, there's a mock server provided. The `tests_run` target is running and shutting down this mock server automatically.
In some situations, you might want to run the mock server by yourself. 
* Manually running the MockServer:
```
    tests/servermock/run_mock_server.sh
```
You can also modify the response from the MockServer in order to check verious scenarious.
* Modifying the response from the MockServer:
```
    curl -X POST "localhost:5000/set_data" -b '{ "My_Network": ["SK_ADNETWORK_ID1","SK_ADNETWORK_ID2"]}'
```

#### Change the service endpoint
Assuming that there's a running service on `localhost:5000` :
 
    export FYBER_SKAD_NETWORKS_SERVER_HOST='http://localhost:5000/'
    
## Build

All the commands must be run from the repository's base folder.
 
####  Prerequisites

* macOS 10.15
* curl
* cmake 3.18
* python 3.8
* clang-format

### Automatic
```
./clean
./build
```
### Manual
```
./clean

cmake -S . -B build
cmake --build build --target format
cmake --build build --target fix-format
cmake --build build --target skad_updater
```
For more information about formatting see [Format.cmake](https://github.com/TheLartians/Format.cmake/blob/master/README.md)

##### Running Tests
###### Prerequisites
1. make sure `curl` is installed.
1. Make sure you have Python 3.8 installed.
1. Create a [virtual env](https://packaging.python.org/guides/installing-using-pip-and-virtual-environments/) under `tests/servermock` and run `pip install -r requirements.txt`.

###### To run the tests:
```
cmake --build build --target tests_run

cd build/tests/
./tests_run
```

### Package
Generates a `tar.gz` file in the `build` directory.  
If `shasum` is present in the system - the valid homebrew formula `skad_undater.rb` file will also be generated.  
```
cmake --build build --target package
```

## Author
Fyber GmbH ( support@fyber.com )
