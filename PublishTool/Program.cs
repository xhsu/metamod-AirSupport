using System.IO.Compression;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;


namespace AirSupportPublish
{
	unsafe static class Resources
	{
		public static List<string> m_rgszResources = new();

		[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
		public static int PrecacheModel(IntPtr psz)
		{
			var sz = Marshal.PtrToStringUTF8(psz);

			if (sz is not null && !m_rgszResources.Contains(sz))
			{
				m_rgszResources.Add(sz);

				if (sz.ToLower().EndsWith(".mdl"))
					AddSoundFromModel(Path.Combine(Program.m_szResourceRootPath, sz));
			}

			return m_rgszResources.Count;
		}

		[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
		public static int PrecacheSound(IntPtr psz)
		{
			var sz = Marshal.PtrToStringUTF8(psz);

			if (sz is not null && !m_rgszResources.Contains(sz))
				m_rgszResources.Add($"sound/{sz}");

			return m_rgszResources.Count;
		}

		[DllImport("Transpiler.dll", EntryPoint = "ReceiveCSharpFnPtr", CharSet = CharSet.Auto, CallingConvention = CallingConvention.StdCall)]
		public static extern void TranspilerInitialize(delegate* unmanaged[Cdecl]<IntPtr, int> pfnPrecacheModel, delegate* unmanaged[Cdecl]<IntPtr, int> pfnPrecacheSound);

		[DllImport("Transpiler.dll", EntryPoint = "Precache", CharSet = CharSet.Auto, CallingConvention = CallingConvention.Cdecl)]
		public static extern void ReadResourcePool();

		[DllImport("Transpiler.dll", EntryPoint = "AddSoundFromModel", CharSet = CharSet.Auto, CallingConvention = CallingConvention.StdCall)]
		public static extern void AddSoundFromModel(string szPath);
	}

	unsafe internal class Program
	{
		public static string m_szResourceRootPath = "";

		static void Main(string[] args)
		{
			if (args.Length != 1)
			{
				Console.WriteLine("One must start this tool with his resource path.");
				return;
			}

			m_szResourceRootPath = args[0];

			Resources.TranspilerInitialize(&Resources.PrecacheModel, &Resources.PrecacheSound);
			Resources.ReadResourcePool();
			Resources.m_rgszResources.Sort();

			Console.ForegroundColor = ConsoleColor.Cyan;
			Console.WriteLine($"Resources Count: {Resources.m_rgszResources.Count}");
			Console.ResetColor();

			Console.Write('\n');

			var szZipPath = Path.Combine(Directory.GetCurrentDirectory(), $"AirSupport-{DateTime.Now:MMM-dd-yyyy}.zip");

			if (File.Exists(szZipPath))
				File.Delete(szZipPath);

			using (var hZipFile = ZipFile.Open(szZipPath, ZipArchiveMode.Create))
			{
				// #TODO change to MACRO somehow?
				hZipFile.CreateEntryFromFile("AirSupport.dll", "addons/metamod/dlls/AirSupport.dll", CompressionLevel.SmallestSize);

				foreach (var szRelativePath in Resources.m_rgszResources)
				{
					var szAbsolutePath = Path.Combine(m_szResourceRootPath, szRelativePath);

					if (File.Exists(szAbsolutePath))
					{
						Console.ForegroundColor = ConsoleColor.DarkGreen;
						Console.Write($"Packing {szRelativePath}");
						Console.ForegroundColor = ConsoleColor.DarkGray;
						Console.Write($"({szAbsolutePath})\n");
						Console.ResetColor();

						hZipFile.CreateEntryFromFile(szAbsolutePath, szRelativePath, CompressionLevel.SmallestSize);
					}
					else
					{
						Console.ForegroundColor = ConsoleColor.DarkRed;
						Console.WriteLine($"Missing {szRelativePath}");
						Console.ResetColor();
					}
				}
			}

			Console.Write($"\nPackage saved as ");
			Console.ForegroundColor = ConsoleColor.Black;
			Console.BackgroundColor = ConsoleColor.White;
			Console.Write(szZipPath);
			Console.ResetColor();
			Console.Write('\n');

			Console.Write("\nDone.\nPress ENTER key to close the window.\n");
			Console.In.ReadLine();
		}
	}
}
