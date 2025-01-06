<h1>Clusters</h1>

<p>
I have written an algorithm, which can order and sort large numbers of items.<br />
It creates a pyramidal directory, moving entries and groups accordingly.<br />
Items can be added, removed and looked-up in constant low time.
</p><br />

<img src="https://github.com/user-attachments/assets/25a5f9b2-fbf7-4be2-85fd-642b8ce59aaf" width="400" /><br />
<br />

<p>
This is the standard-implementation, there are two more implementations in <a href="https://github.com/svenbieg/Heap">C</a> and <a href="https://github.com/svenbieg/Clusters.NET">C#</a>.
</p>
<br />

<h2>Principle</h2>
<br />

<table>
	<tr>
		<td><img src="https://github.com/user-attachments/assets/4901aa2d-e7a3-4367-aab3-cd1a7827de4f" width="50" align="right" /></td>
		<td>The entries are stored in groups.</td>
	</tr><tr><td></td></tr><tr>
		<td><img src="https://github.com/user-attachments/assets/ad42262d-21e7-4749-a099-f22a6c61671d" width="50" align="right" /></td>
		<td>The size of the groups is limited and 10 by default.</td>
	</tr><tr><td></td></tr><tr>
		<td><img src="https://github.com/user-attachments/assets/ac228371-af19-45e8-a9f6-7758389612c5" width="100" align="right" /></td>
		<td>If the group is full a parent-group is created.</td>
	</tr><tr><td></td></tr><tr>
		<td><img src="https://github.com/user-attachments/assets/4ec67147-7d1c-40fd-9226-54c66f51f365" width="100" align="right" /></td>
		<td>The first and the last entry can be moved to the neighbour-group.</td>
	</tr><tr><td></td></tr><tr>
		<td><img src="https://github.com/user-attachments/assets/f4f968e2-2065-4979-bd5e-40edd734ef5b" width="100" align="right" /></td>
		<td>The entries are moved between the groups, so all groups get as full as possible.</td>
	</tr><tr><td></td></tr><tr>
		<td><img src="https://github.com/user-attachments/assets/523f36fd-bdfe-4978-9fff-b5975829c00e" width="150" /></td>
		<td>The number of groups is limited too, another parent-group is created.</td>
	</tr><tr><td></td></tr><tr>
		<td><img src="https://github.com/user-attachments/assets/624247eb-18b7-4247-8c56-eaccc59c2340" width="150" /></td>
		<td>If an entry needs to be inserted in a full group, a whole sub-tree can be moved.</td>
	</tr>
</table><br />

<p>
You can find detailed information in the
<a href="https://github.com/svenbieg/Clusters/wiki/Home">Wiki</a>.
</p>
