## Windows duplicate file detection helper

1. Use Windows _dir_ command to generate a .txt file with list of files, paths and filesizes
2. order by filesize
3. find "clusters" of files with same size
4. create output file with:<br>
&emsp;Files of size abc:<br>
&emsp;&emsp;&lt;List of paths&gt;<br>
&emsp;Files of size xyz:<br>
&emsp;&emsp;&lt;List of paths&gt;<br>
&emsp;...

<b>Ideas, Goals</b>
* experimenting with calls to system functions
* testing ways to display progress in console for long running processes<br>
&emsp;&emsp;eg "rotating bar" (/)&rarr; (-)&rarr; (\\)&rarr; (|) or "moving ellipsis" (.  )&rarr; (.. )&rarr; (...)<br>

(TODO losts of cleanup)
