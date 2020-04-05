open UI;
open BsReactIcons;

module Location = {
  [@bs.val] external pathname: string = "location.pathname";
  [@bs.val] external origin: string = "location.origin";
};

module History = {
  [@bs.val]
  external pushState: (option(string), string, string) => unit =
    "history.pushState";
};

[@react.component]
let make = () => {
  let (location, setLocation) = React.useState(() => Location.pathname);
  let (content, setContent) = React.useState(() => None);

  React.useEffect1(
    () => {
      History.pushState(None, "", Location.origin ++ location);

      API.getFolder(location)
      |> Js.Promise.then_((res: API.response) => {
           setContent(_ => Some(res));
           Js.Promise.resolve();
         })
      |> Js.Promise.catch(_ => {
           setContent(_ => None);
           Js.Promise.resolve();
         })
      |> ignore;
      None;
    },
    [|location|],
  );

  <div>
    <nav className="bg-white flex items-center p-4 shadow">
      <span> "UI-explorer"->React.string </span>
    </nav>
    <div className="max-w-screen-lg mx-auto mt-8">
      {content->Option.mapWithDefault(
         React.null, ({content, breadcrumb, folderPath}) => {
         <>
           <div className="mb-4 flex align-items">
             <ul className="flex items-center ml-4">
               <li>
                 <button onClick={_ => setLocation(_ => "/")}>
                   <TiHome size=28 />
                 </button>
               </li>
               {breadcrumb
                ->Array.mapWithIndex((i, b) => {
                    <li key={"breadcrumpb-" ++ i->string_of_int}>
                      <span className="mx-4"> "/"->React.string </span>
                      <button onClick={_ => setLocation(_ => b.value)}>
                        b.label->React.string
                      </button>
                    </li>
                  })
                ->React.array}
             </ul>
           </div>
           <div className="flex flex-wrap flex-col mx-2">
             {content
              ->Array.mapWithIndex((index, item) =>
                  <div
                    key={index->string_of_int}
                    className="w-full justify-between p-4 border-b border-solid border-gray-600 flex last-child:border-b-0 items-center text-xl ">
                    <div className="flex items-center">
                      <span className="mr-4">
                        {item.isDirectory
                           ? <MdFolder size=30 /> : <FiFile size=28 />}
                      </span>
                      {item.isDirectory
                         ? <button
                             title={item.name}
                             className="hover:underline"
                             onClick={_ => {
                               setLocation(currentLocation => {
                                 switch (currentLocation) {
                                 | "/" => currentLocation ++ item.name
                                 | v => v ++ "/" ++ item.name
                                 }
                               })
                             }}>
                             <span className="truncate">
                               item.name->React.string
                             </span>
                           </button>
                         : item.name->React.string}
                    </div>
                    <Dropdown
                      button={toggle =>
                        <Button onClick={_ => toggle()}>
                          <FiMoreVertical />
                        </Button>
                      }
                      elements=[|
                        <Dropdown.Item>
                          <FiDownload />
                          <a
                            href={
                              "http://localhost:4444/download"
                              ++ folderPath
                              ++ item.name
                            }
                            className="ml-2"
                            download="true">
                            "Download"->React.string
                          </a>
                        </Dropdown.Item>,
                        <Divider />,
                        <Dropdown.Item>
                          <FiPlayCircle />
                          <span className="ml-2">
                            "Stream"->React.string
                          </span>
                        </Dropdown.Item>,
                        <Divider />,
                        <Dropdown.Item className="text-red-500">
                          <FiTrash2 />
                          <span className="ml-2">
                            "Delete"->React.string
                          </span>
                        </Dropdown.Item>,
                      |]
                    />
                  </div>
                )
              ->React.array}
           </div>
         </>
       })}
    </div>
  </div>;
};